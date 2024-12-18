# Documenation

## Guidelines for contributors

- Do not use `clang-format` (we already had our disasters with it)
- Try to keep ident the same as the rest of the project
- Use `pdqsort` for simulation/game code, use `sys::merge_sort` for UI code (or code where sorting may be unstable)
- Keep the simulation and game code deterministic, otherwise you will face consequences (such as OOS)
- UI code can be non deterministic, be aware that the simulation runs parallel to the UI code, so values can change in-between reads
- Try to minimize the number of memory reads, and maximize the number of "local" registers used
- Minimize the amount of register pressure (you have to guess how many "pressure" you're applying)
- Try to keep your data aligned to a cache line, or even better, make it exactly fit a cache line, or half a cache line
- Or otherwise, keep your data in segments that wouldn't cause "extra" cache lines to be prefetched for no reason
- Keep cache locality at it's maximum, i.e prefetch elements 1, 2, 3... instead of 1, 8, 16, 24...
- Do not use intrinsics, instead use the ve library (it has NEON and RISC-V vector intrinsics, and cross-platform)
- Try to use ternaries, if/else is okay too, but understand that if the compiler can synthetize your expression from `1 < x ? 3 : 6` to `int arr[2] = { 6, 3 }; arr[1 < x]` then you are good.
- Keep cache locality!
- Remember DCON is a giant database, treat it as such, every query you do incurs an associated cost (for memory prefetch). So if you have to absolutely read something, try to read it in a way that minimizes reads: for example, "eveyr army of a nation" can be done by querying `nation -> army_control -> army` (3 reads), instead of `all armies -> if army -> army_control -> controller == nation` (5 reads)
- Usage of `_mm_extract_ps` is not allowed, see here: https://community.intel.com/t5/Intel-Moderncode-for-Parallel/mm-extract-ps-returns-int-for-a-long-long-time/td-p/1145389
- Try to keep your code "simple" (no need for a complex solution)

## Optimizing

### Memory and "cache pressure"

In a program, you usually want to do as few writes as possible, or, make it so that reads and writes only occur at specific points, while the intermediate results are computed using registers. Ideally.

The issue with the following code demostrates why:

```c++
for(auto sc : state.world.nation_get_state_ownership_as_nation(n)) {
	auto location = sc.get_state();
	state.world.state_instance_set_demographics(location, key, 0.f);
	for(auto sm : sc.get_state().get_definition().get_abstract_state_membership()) {
		state.world.state_instance_get_demographics(location, key) += state.world.province_get_demographics(sm.get_province(), key);
	}
}
```

Alright, so compilers are smart, very smart. But sometimes they can't see that "state_instance_get_demographics" will return a reference to a memory area, that could be optimized away - mainly due to `vptr`. The result? Lots of writes and reads for no reason, at every iteraion.

A better version would be as follows, using an explicit temporal variable for summation (that is most likely converted into a register). This results in the same amount of potential memory reads, but now with a reduced amount of writes (only 1).

```c++
for(auto sc : state.world.nation_get_state_ownership_as_nation(n)) {
	auto location = sc.get_state();
	float sum = 0.f;
	for(auto sm : sc.get_state().get_definition().get_abstract_state_membership()) {
		sum += state.world.province_get_demographics(sm.get_province(), key);
	}
	state.world.state_instance_set_demographics(location, key, sum);
}
```

This is not to say writes are slow, just unescesary, the CPU will cache this entire line containing the underlying array for the setter/getter. However, it is important to recognize the fastest cache are the registers. So while small, this can make the difference in very bottlenecked code (summation of demographics for example). Especially if the compiler is unable to see thru translation units.

### Branchless

Compilers are pretty good at optimizing your code, before trying to jump into removing every `if` and ternary from KE, stop for a moment and realize that branches are not that bad. We are not in a GPU where a branch kills your performance (if both paths dont take the same time, for example).

### Threading

Use threading when you know that the operations will take all roughly the same time. Concurrency is delicate, any thread blocking the others will stall the entire execution line until every thread finishes. It is paramount the time consumed by the thread groups doesn't exceed that of the single threaded approach. Remember thread dispatching also implies an overhead.

### SIMD

Take the following expression:

```c
typedef struct {
	int min, max;
} result;
result min_max(int a[], int n) {
	result r = { 0, 0 };
	for(int i = 0; i < n; ++i) {
		if(r.min > a[i]) {
			r.min = a[i];
		}
        if(r.max < a[i]) {
			r.max = a[i];
		}
	}
	return r;
}
```

The compiler may make something similar to the following assembly:

```sh
#rdi = int a[]
#rsi = int n
#rax = { int a, int b } (int is 32-bit, packed on 64-bit register)
min_max:
	testl %esi, %esi			# test, less than (if esi is 0, this skips to the end)
	jle .L4						# jump if the above test (less than) returned true
	movq %rdi, %rcx				# store rdi into rcx (to use as pointer, below)
	movslq %esi, %rsi			# sign extend move: 32-bit -> 64-bit
	leaq (%rdi,%rsi,4), %rdi	# compute address (rdi + rsi * 4) into rsi (NOT A LOAD)
	movl $0, %esi
	movl $0, %eax
.L3:							#
	movl (%rcx), %edx			# move address of [rcx] into edx
	cmpl %edx, %eax				# compare less than, edx and eax (0)
	cmovg %edx, %eax			# cond. move if greater than: edx > eax
	cmpl %edx, %r8d				# compare less than edx, r8d
	cmovl %edx, %r8d			# cond. move if less than: edx < r8d
	addq $4, %rcx				# add 4 to rcx
	cmpq %rdi, %rcx				# compare if we hit the end (rsi stores the end of array)
	jne .L3						# no? then go and do all of this again (into L3)
.L2:							#
	salq $32, %r8				# shift r8 to the left 32 bits
	movl %eax, %eax				# nop
	orq %r8, %rax				# then or the (shifted) r8 into rax to form the final result
	ret							#
.L4:
	movl $0, %esi
	movl $0, %eax
	jmp .L2
```

```
__m128i _mm_minpos_epu16(__m128i a) {
	uint16_t index = 0;
	uint16_t min = a.u16[0];
	for(uint32_t j = 0; j <= 7; ++j) {
		uint32_t i = j * 16;
		if(a.u16[j] < min) {
			index = j;
			min = a.u16[j];
		}
	}
	__m128i dst;
	dst.u16[0] = min;
	dst.u16[1] = index;
	return dst;
}
```

### SIMD Example #1

Take a look at this code:

```c++
auto ln_max = state.world.pop_type_make_vectorizable_float_buffer();
auto en_max = state.world.pop_type_make_vectorizable_float_buffer();
auto lx_max = state.world.pop_type_make_vectorizable_float_buffer();
state.world.for_each_pop_type([&](dcon::pop_type_id pt) {
	float ln_total = 0.0f;
	float en_total = 0.0f;
	float lx_total = 0.0f;
	for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
		dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
		auto kf = state.world.commodity_get_key_factory(c);
		if(state.world.commodity_get_is_available_from_start(c)
		|| (kf && state.world.nation_get_active_building(n, kf))) {
			auto sat = state.world.nation_get_demand_satisfaction(n, c);
			auto ln_val = state.world.pop_type_get_life_needs(pt, c) * pop_get_life_needs_weight(state, n, c);
			ln_total += ln_val;
			ln_max.get(pt) += ln_val * sat;
			auto en_val = state.world.pop_type_get_everyday_needs(pt, c) * pop_get_everyday_needs_weight(state, n, c);
			en_total += en_val;
			en_max.get(pt) += en_val * sat;
			auto lx_val = state.world.pop_type_get_luxury_needs(pt, c) * pop_get_luxury_needs_weight(state, n, c);
			lx_total += lx_val;
			lx_max.get(pt) += lx_val * sat;
		}
	}
	if(ln_total > 0.f) ln_max.get(pt) /= ln_total;
	else ln_max.get(pt) = 1.f;
	if(en_total > 0.f) en_max.get(pt) /= en_total;
	else en_max.get(pt) = 1.f;
	if(lx_total > 0.f) lx_max.get(pt) /= lx_total;
	else lx_max.get(pt) = 1.f;
});
```

Relatively simple, aye? We do a sum over 3 different values (ouch) and we additionally do a bunch of operations, all of this by N times (number of pop types). Double ouch.

Let's start with memory. We are taking by reference (`ln_max.get()`) and summing it, this is fine for small loops, but the vectorizable buffer may not entirely fit on a cache line. So let's delay our sum:

```c++
	float ln_value = 0.0f;
	float en_value = 0.0f;
	float lx_value = 0.0f;
	//...
			auto sat = state.world.nation_get_demand_satisfaction(n, c);
			auto ln_val = state.world.pop_type_get_life_needs(pt, c) * pop_get_life_needs_weight(state, n, c);
			ln_total += ln_val;
			ln_value += ln_val * sat;
			auto en_val = state.world.pop_type_get_everyday_needs(pt, c) * pop_get_everyday_needs_weight(state, n, c);
			en_total += en_val;
			en_value += en_val * sat;
			auto lx_val = state.world.pop_type_get_luxury_needs(pt, c) * pop_get_luxury_needs_weight(state, n, c);
			lx_total += lx_val;
			lx_value += lx_val * sat;
	//...
	if(ln_total > 0.f) ln_max.get(pt) = ln_value / ln_total;
	else ln_max.get(pt) = 1.f;
	if(en_total > 0.f) en_max.get(pt) = en_value / en_total;
	else en_max.get(pt) = 1.f;
	if(lx_total > 0.f) lx_max.get(pt) = lx_value / lx_total;
	else lx_max.get(pt) = 1.f;
```

Another hindsight is those function calls, their definition is literally the repetition of the above condition for commodities (is available from start, has active buildings) -- but with the extra "is_life_need", so we can take those out too (it will make sense later why). We will also explicitly convert the result into a float via the use of a ternary, instead of relying on the usual `boolean -> int -> float` conversion, we directly do `boolean -> float`.

```c++
			auto sat = state.world.nation_get_demand_satisfaction(n, c);
			auto ln_val = state.world.pop_type_get_life_needs(pt, c)
				* (state.world.commodity_get_is_life_need(c) ? 1.f : 0.f);
			ln_total += ln_val;
			ln_value += ln_val * sat;
			auto en_val = state.world.pop_type_get_everyday_needs(pt, c)
				* (state.world.commodity_get_is_everyday_need(c) ? 1.f : 0.f);
			en_total += en_val;
			en_value += en_val * sat;
			auto lx_val = state.world.pop_type_get_luxury_needs(pt, c)
				* (state.world.commodity_get_is_luxury_need(c) ? 1.f : 0.f);
			lx_total += lx_val;
			lx_value += lx_val * sat;
```

The last section may look particularly unfriendly, however it's relatively simple to observe the comparison at hand:
```c++
	if(ln_total > 0.f) ln_max.get(pt) = ln_value / ln_total;
	else ln_max.get(pt) = 1.f;
	if(en_total > 0.f) en_max.get(pt) = en_value / en_total;
	else en_max.get(pt) = 1.f;
	if(lx_total > 0.f) lx_max.get(pt) = lx_value / lx_total;
	else lx_max.get(pt) = 1.f;

	// becomes
	ln_max.set(pt, ln_total > 0.f ? ln_value / ln_total : 1.f);
	en_max.set(pt, ln_total > 0.f ? ln_value / ln_total : 1.f);
	lx_max.set(pt, ln_total > 0.f ? ln_value / ln_total : 1.f);
```

We observe that indeed, division by 0 wants to be avoided in this case, so we default to... 1. That's surely a way to interpret mathematics.

#### Equivalences

Instead of writing intrinsics yourself, we use a framework called DataContainer, that provides a module `ve`, this `ve` module adds C++ syntatic sugar to otherwise magic intrinsics (`_mm256_add_ps` is used for `operator+` for example).

Herein, the following are equivalences:

- `cond ? a : b = ve::select(cond, a, b)`
- `std::max(a, b) = ve::max(a, b)`
- `std::min(a, b) = ve::min(a, b)`

The typical data types we use is `ve::fp_vector` (float), `ve::int_vector` (int32_t) and `ve::mask_vector` (boolean/comparison).

So it is quite easy to transform our code:

```c++
auto ln_max = state.world.pop_type_make_vectorizable_float_buffer();
auto en_max = state.world.pop_type_make_vectorizable_float_buffer();
auto lx_max = state.world.pop_type_make_vectorizable_float_buffer();
state.world.execute_serial_over_pop_type([&](auto ids) {
	// Optimal values (100% satisfaction of goods)
	ve::fp_vector ln_optimal(0.f);
	ve::fp_vector en_optimal(0.f);
	ve::fp_vector lx_optimal(0.f);
	// Actual perceived satisfaction (how much was actually satisfied in reality)
	ve::fp_vector ln_value(0.f);
	ve::fp_vector en_value(0.f);
	ve::fp_vector lx_value(0.f);
	for(uint32_t i = 1; i < state.world.commodity_size(); ++i) {
		dcon::commodity_id c{ dcon::commodity_id::value_base_t(i) };
		auto kf = state.world.commodity_get_key_factory(c);
		if(state.world.commodity_get_is_available_from_start(c)
		|| (kf && state.world.nation_get_active_building(n, kf))) {
			auto sat = state.world.nation_get_demand_satisfaction(n, c);
			//
			auto ln_val = state.world.pop_type_get_life_needs(ids, c)
				* (state.world.commodity_get_is_life_need(c) ? 1.f : 0.f);
			ln_value = ln_value + ln_val;
			ln_optimal = ln_optimal + ln_val * sat;
			//
			auto en_val = state.world.pop_type_get_everyday_needs(ids, c)
				* (state.world.commodity_get_is_everyday_need(c) ? 1.f : 0.f);
			en_value = en_value + en_val;
			en_optimal = en_optimal + en_val * sat;
			//
			auto lx_val = state.world.pop_type_get_luxury_needs(ids, c)
				* (state.world.commodity_get_is_luxury_need(c) ? 1.f : 0.f);
			lx_value = lx_value + lx_val;
			lx_optimal = lx_optimal + lx_val * sat;
		}
	}
	ln_max.set(ids, ve::select(ln_value > 0.f, ln_optimal / ln_value, 1.f));
	en_max.set(ids, ve::select(en_value > 0.f, en_optimal / en_value, 1.f));
	lx_max.set(ids, ve::select(lx_value > 0.f, lx_optimal / lx_value, 1.f));
});
```

We simply exchanged `float` for `ve::fp_vector`, we also don't have an overloaded `operator+=`, so we explicitly do it ourselves (remember, `a += b` is `a = a + b`). And finally the ternary can be simplified to a `ve::select`:

```c++
	ln_max.set(pt, ln_total > 0.f ? ln_value / ln_total : 1.f);
	en_max.set(pt, ln_total > 0.f ? ln_value / ln_total : 1.f);
	lx_max.set(pt, ln_total > 0.f ? ln_value / ln_total : 1.f);

	//becomes
	ln_max.set(ids, ve::select(ln_value > 0.f, ln_optimal / ln_value, 1.f));
	en_max.set(ids, ve::select(en_value > 0.f, en_optimal / en_value, 1.f));
	lx_max.set(ids, ve::select(lx_value > 0.f, lx_optimal / lx_value, 1.f));
```

Note that we didn't SIMD the commodities, this is because we can't address multiple objects as vectors at the same time. `state.world.pop_type_get_life_needs(ids, c)` will not work if both `ids` and `c` are vectors - as the system wasn't designed to handle this kind of input.

For now this caveat prevents some major optimizations from being done. Both by the compiler and the programmer.

### Non-determinism

See this code, see anything wrong?

```c++
// combination and final execution
auto total_vector = decisions_taken.combine([](auto& a, auto& b) {
	std::vector<decision_nation_pair> result(a.begin(), a.end());
	result.insert(result.end(), b.begin(), b.end());
	return result;
});
```

Exactly, it's missing total ordering. This will cause Out-Of-Synch errors, non-determinism and you will get angry victorians yelling "why the fuck is this OOSing". So remember, to always, sort, in a total order-deterministic fashion:

```c++
// ensure total deterministic ordering
pdqsort(total_vector.begin(), total_vector.end(), [&](auto a, auto b) {
	auto na = a.second;
	auto nb = b.second;
	if(na != nb)
		return na.index() < nb.index();
	return a.first.index() < b.first.index();
});
```

## Project guidelines

### Release numbering

KE uses four numbers for telling the version of KE: x.y.z.t. x will be only increase if we the devs believe that KE is release quality. y will be increased after a lot of change happend to KE (usually 4-6 months of changes). And z increases when we are confident that this version is better than the previous one. t is only used if we want to signal that we aren't sure that KE has improved from the previous version enough for z to be increased. Meaning that if we use t it signals that this version may have unfinished, not tested features.

So both x.y.z and x.y.z.t is a valid version. Only difference is that the first one is a release and the second one is a pre release.

The following is an example of the system:
0.1.0
0.1.1.1
0.1.1.2
0.1.1.3
0.1.1
0.1.2.1
0.1.2.2
0.1.2

### `filter` vs `mask`

What's the difference? Well in simple terms, look at this SIMD code:

```c++
// Promotion -- national focus
auto const mask_a = promoting && nf_ptype != dcon::pop_type_id{} && nf_strata >= strata
	&& (is_state_capital || state.world.pop_type_get_state_capital_only(nf_ptype) == false);
// Demotion -- national focus
auto const mask_b = !promoting && nf_ptype != dcon::pop_type_id{} && nf_strata <= strata
	&& (is_state_capital || state.world.pop_type_get_state_capital_only(nf_ptype) == false);
// General passed filter
auto const filter_a = owners != dcon::nation_id{} && promotion_chances > 0.f && demotion_chances > 0.f
	&& base_amounts > 0.f;
```

A filter is to be given as a parameter to a `ve::apply` to be serially applied to a set of instructions as a form of a predicate. Whereas a mask is used within SIMD code to mask off elements without a `ve::apply`.
