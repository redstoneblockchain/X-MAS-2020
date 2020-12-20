# Help a Santa helper?

Hey, I found some secret server on which Krampus stores his private details.  
It seems like he has some kind of difficult crypto challenge instead of a login, if we pass that we should get access to valuable information.  
Let's give it a try, what do you say?

**Target:** nc challs.xmas.htsp.ro 1004
**Author:** Gabies  
**Category:** Cryptography
**Link (original):** https://drive.google.com/drive/folders/1Ec-Nt1JcZyKwZDEaI8mM6e_80RBZEVuW?usp=sharing
**Link (cached):** https://github.com/redstoneblockchain/X-MAS-2020/tree/master/help-a-santa-helper/files

![image](https://user-images.githubusercontent.com/6524684/102717772-c3ce8380-42e4-11eb-9be3-99a7d31fb548.png)

---

The challenge begins (as most crypto challenges here) with a proof of work.
Thankfully, the [`shapow`](https://github.com/krisives/shapow) Kris wrote last year had me covered there.

Upon passing the PoW, we get:

```
Hello, thanks for answering my call, this time we have to break into Krampus' server and recover a stolen flag.
We have to solve a hash collision problem to get into the server.
Sadly, we're on a hurry and you only have 2 actions you can make until we get detected.
Choose what you want to do:
1. hash a message
2. provide a collision
3. exit
```

Choosing `1` allows you to provide a message and obtain its hash.

Choosing `2` means taking a stab at solving the challenge by virtue of providing two (different) messages that
have the same hash.

Choosing `3` prints `Sad to see you leave, I was eager to get that flag.` (for completeness).

The provided files include the proof of work code, the server which holds no surprises, and the hash function.
The latter is implemented in `chall.py`, reproduced here for convenience:

```python
import os
from Crypto.Cipher import AES
from binascii import hexlify

def xor(a, b):
    return bytes([x^y for x,y in zip(a, b)])

def pad(msg, block_size):
    if len(msg) % block_size == 0:
        return msg
    return msg + bytes(block_size - len(msg) % block_size)

class Hash:
    def __init__(self, seed = None):
        if seed == None:
            seed = os.urandom(16)

        self.perm = AES.new(seed, AES.MODE_ECB)
        self.get_elem = self.perm.encrypt
        self.hash = bytes(16)
        self.string = b""

    def update(self, msg):
        msg = pad(msg, 16)
        for i in range(0, len(msg), 16):
            self.string += msg[i:i+16]
            self.hash = xor(msg[i:i+16], self.get_elem(xor(self.hash, msg[i:i+16])))

    def digest(self):
        return self.hash

    def hexdigest(self):
        return hexlify(self.digest())
```

Ok, we can `xor` bytes and messages are zero-padded. The hasher itself contains:

- AES in ECB mode, which Wikipedia tells me (yes, my crypro is weak) means that identical message blocks are encrypted identically (!).

- The hash state: 16 bytes that are initially zero (!).

- The original message (used by the server to check that the colliding messages are actually distinct).

The real meat lies in the `update` function, which hashes the message in blocks of 16 bytes. We can safely assume that `self.get_elem`
is a one-way function, so clearly we have to make something clever out of the `xor` of `self.hash` and the current message block.

Let us consider what happens when `msg` is `0` (well, `00` to be a valid hex byte string):

<img src="https://render.githubusercontent.com/render/math?math=h(state=0, msg=0) = 0 \oplus AES(0 \oplus 0) = AES(0)">

Ok, so if we input `0` as message, we get `AES(0)` as the resulting hash state (and final hash). Well, our only real chance of
producing a collision is to feed the same input to `get_elem` twice, so let's have `state = AES(0)` and `msg[i:i+16] = AES(0)`.
We can achieve the former by having the first message block be `0` and the latter by requesting the hash for message `0` beforehand.

<img src="https://render.githubusercontent.com/render/math?math=h(state=AES(0), msg=AES(0)) = AES(0) \oplus AES(AES(0) \oplus AES(0)) = AES(0) \oplus AES(0) = 0">

Well, that's it! Any message starting with 16 zero bytes followed by the hash of those zero bytes  brings us back to the
initial state of the hasher. The solution is thus to choose `1`, hash the hex string `00` with result `h0`, then choose `2` and provide
an arbitrary string `x` as well as `000000000000000000000000000000<h0><x>` to produce a collision.

![image](https://user-images.githubusercontent.com/6524684/102719036-d304ff80-42eb-11eb-89fa-77924f53d533.png)

---

According to Gabies and trupples on Discord [this NSUCRYPTO paper](
https://cdn.discordapp.com/attachments/520005746457575435/789598299376189450/writeup-8.pdf) of theirs details the
approach and the math that inspired this challenge. It feels slightly more convoluted than my solution above,
but I haven't put in the effort to understand whether the problem tackled there is of a more general nature.
