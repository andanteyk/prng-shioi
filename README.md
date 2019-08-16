# `shioi` - fast pseudorandom number generator

[Japanese version is here. / 日本語はこちら](README-ja.md)

## Minimal Implementation (C)

```c
uint64_t rotl(uint64_t x, int k) { return (x << k) | (x >> (-k & 63)); }
// State must be initialized with any value except {0, 0}
uint64_t next(uint64_t state[2]) {
	uint64_t s0 = state[0], s1 = state[1];
	uint64_t result = rotl(s0 * 0xD2B74407B1CE6E93, 29) + s1;
	state[0] = s1;
	state[1] = (s0 << 2) ^ ((int64_t)s0 >> 19) ^ s1;
	return result;
}
// equivalent to 2^64 next() calls
void jump64(uint64_t state[2]) {
	uint64_t s0 = state[0], s1 = state[1];
	state[0] = s0 ^ s1;
	state[1] = (s0 << 2) ^ ((int64_t)s0 >> 19);
}
```

For more details, see `shioi128.c`.

## Pros

1. It is fast in 64 bit environment.
	* About 3.1 times faster than Mersenne Twister(64 bit version).
1. It is portable and easy to implement.
	* It does not use environment / language dependent instructions such as 128 bit multiplication.
1. Its period is 2^128 - 1, which is mathematically provable.
	* It is sufficient for games and simulations.
1. The output is 1-dimensionally equidistributed. All 64-bit integer values are output with almost equal probability.
1. There are no low quality bits in the output.
	* As with LCG, there is no problem that the lower bits have low randomness.
1. It is space efficient.
	* It uses only 128 bits == 16 bytes.
1. It passes many powerful randomness tests.  
	(In the following, "rev" represents the output bit order reversed, and "std" represents the output as it is.)
	* [PractRand](http://pracrand.sourceforge.net/) v0.94 expanded extra ( `-tf 2 -te 1` ) 32TB: no anomalies in 2417 test result(s)
	* [Hamming-weight dependencies](http://prng.di.unimi.it/hwd.php) 1.5PB: p = 0.356
	* [TestU01](http://simul.iro.umontreal.ca/testu01/tu01.html) v.1.2.3 BigCrush std/rev: In all tests, p in [1e-10, 1 - 1e-10]
	* [gjrand](http://gjrand.sourceforge.net/) v4.2.1.0
		* testunif 10T ( `--ten-tera` ): p = std 0.831 / rev 0.283
		* testfunif 1T ( `--tera` ): p = std 0.526 / rev 0.448
1. It has a fast jump function available. It makes it easy to parallel execution.
	* An operation equivalent to 2^64 `next()` calls can be performed in same amount of time as `next()`.
	* It provides 2^64 non-overlapping sequences of length 2^64.

## Cons

1. It is not cryptographically secure pseudorandom number generator.
	* DO NOT use for cryptographic purposes.
	* I tried to restore the internal state from 3 consecutive raw 64-bit outputs using [Z3 solver](https://github.com/Z3Prover/z3), but failed in a day.
1.  There are variants that use 32 bit words, but the investigation is incomplete.
1. 64-bit constant multiplication is required. In an environment where this is slow, output speed may decrease.
1. Because it is new, it has not been investigated by others.

## Comparison

Comparison with major (64 bit output) pseudorandom number generators:

|Name|Period|Size(bytes)|Equidistribution|Jump|Failed Test|Speed(64bit/ns)|
|:---|---:|---:|---:|:---|:---|---:|
|[`sfc64`](http://pracrand.sourceforge.net/)|> 2^64|32|0|-|-|1.21|
|`seiran`|2^128 - 1|16|1|✔|-|1.20|
|[`xoroshiro128+`](http://xoshiro.di.unimi.it/xoroshiro128plus.c)|2^128 - 1|16|1|✔|BRank, hwd|1.13|
|👉 `shioi`|2^128 - 1|16|1|✔|-|1.00|
|[`xoshiro256**`](http://xoshiro.di.unimi.it/xoshiro256starstar.c)|2^256 - 1|32|4|✔|-|0.99|
|[`lehmer128`](https://web.archive.org/web/20180609040734/http://xoshiro.di.unimi.it/lehmer128.c) (LCG)|2^126|16|1|✔|TMFn|0.74|
|[`splitmix`](http://xoshiro.di.unimi.it/splitmix64.c)|2^64|8|1|✔|-|0.68|
|[`pcg64_xsh_rr`](http://www.pcg-random.org/)|2^128|16|1|✔|-|0.38|
|[`mt19937_64`](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/VERSIONS/C-LANG/mt19937-64.c) (Mersenne Twister)|2^19937 - 1|2500|311|✔ (slow)|BRank|0.32|
|[`tinymt64`](http://www.math.sci.hiroshima-u.ac.jp/~m-mat/MT/TINYMT/index-jp.html)|2^127 - 1|16|1|✔|BRank, hwd|0.24|

[Harness used in xoshiro/xoroshiro](http://xoshiro.di.unimi.it/harness.c) was used for speed measurement. The measurement environment is `Intel(R) Core(TM) i7-7700HQ CPU @ 2.80GHz / gcc 7.3.0`.

## License

[Public Domain (CC0)](https://creativecommons.org/publicdomain/zero/1.0/)
