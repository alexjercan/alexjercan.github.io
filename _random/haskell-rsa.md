---
title: Haskell RSA
layout: post
tags: [haskell, security, math]
date: 11 Oct 2023
comments: true
---

# About

RSA implementation in Haskell using
<https://en.wikipedia.org/wiki/RSA_(cryptosystem)>

<div align="center">
  <img src="/images/haskell-rsa/example.png" width="512"/>
  <div align="center">Example Results</div>
  <br/>
</div>

# Usage

`λ> (public, private) = keygen Int Int`

`λ> encrypt public [Integral]`

`λ> decrypt private [Integral]`

### Key Generation

n is part of the public key and the private key
e is part of the public key
d is part of the private key

keygen returns a tuple formed by the public and private keys
public = (n, e)
private = (n, d)

1. n is the product of two primes
2. λ(n) is computed using Carmichael's totient function
but in this case since p and q are primes we can use
the lcm formula
3. e is computed such that λ(n) and e are coprime
4. d is equal to the modular multiplicative inverse of λ(n)


### Primes Generation

1. Write down the list 2,3...
2. Mark the first value as prime
3. Remove multiples of p from the list
4. Return to step 2

# Conclusion

- [GitHub Repo](https://github.com/alexjercan/haskell-rsa)
