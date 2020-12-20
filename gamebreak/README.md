# GAMEBREAK + Demo party

The greatest step towards the popularization of the EMU 1.0 was its entry into the home entertainment market, starting with the release
of the Pluton GAMEBREAK extension. We don't have much to go on but a crumpled up reference card from a developer, a picture of a retail box,
a ROM that supposedly uses the graphics and memory extensions, and a reconstructed picture of what it should display.

The external memory seemed to have had 256k slots which are 18-bit addresable. The old screens are 64x64 with 6 bit color (2 bits per channel),
and the Joysticks have an 8-direction stick and two buttons (X, Y).

Implement the GAMEBREAK devices and make a nice demo to get some points!

**Author:** trupples, Milkdrop  
**Category:** Emulation  
**Files (original):** https://drive.google.com/drive/folders/1YTteUSsK0LCShsj4huFiAaIwAOqsgHsC?usp=sharing  
**Files (cached):** https://github.com/redstoneblockchain/X-MAS-2020/tree/main/gamebreak/files/  

![image](https://user-images.githubusercontent.com/6524684/102721104-f71b0d80-42f8-11eb-9453-96b20d49cc44.png)

---

**Link to final demo:** https://thumbs.gfycat.com/ThoughtfulJaggedChipmunk-mobile.mp4

Another year, another demo party!

Last year we made our demo a rickroll. While not exactly in the spirit of demos, it was still very challenging to fit as much video data as possible
into the very limited memory of the CHIP9. This year there were essentially no size restrictions because the tapes fed to the EMU 1.0 could be of
infinite length in principle, so you could dump any video you want in there. Sure, you still have to code it, but as a concept for a demo I found that
entirely boring.

Instead, the scribble in the EMU 1.0 manual *"Fun: implement some wireframe rendering"* was always in the back of my mind. But that alone
would be kind of dull, and I didn't particularly look forward to just implementing Bresenham. So I set out to actually 3D render a cube.
And what better cube to render for my team's demo than a block of redstone? Or, you know, a **Redstone Blockchain**? :)

I had no intentions of implementing sin/cos on EMU 1.0 (I mean, it's not impossible, but 6 bit fixed point is... limiting). I settled quickly on
having the transformations hardcoded into the rom. This is a pretty neat solution actually, because for each face of the cube you only need to
store a single transformation matrix (from screen coordinates to plane uv coordinates), and if the uv coordinates are within the correct range
you just do a straightforward texture lookup. You don't even need triangles! And if you use an orthogonal projection (which you have to, unless
you want to write a divider for dividing by z, somehow), the matrix has only 6 relevant entries you need to store:

`u = x * m00 + y * m01 + m02`
`v = x * m10 + y * m11 + m12`

The orthogonal projection further means that you can always see exactly 3 faces of a cube: For each pair of opposite faces, only one is visible
at a time. So for the demo, I don't need to store planes for 6 faces at any given time point, only for the three visible ones. Importantly, this
also eliminates the need for culling or z buffering! The three visible faces of a cube cannot occlude each other, so I only need to store different
cubes in the right order (and just fill every pixel based on the *first* face = matrix that leads to valid u, v).

So this was the plan:

1. Create texture and convert it to EMU 1.0 instructions.
2. Implement x, y -> u, v matrix multiplication in EMU 1.0.
3. Write script for computing matrices for the demo and converting them to EMU 1.0 instructions.
4. Create demo by specifying centers, sizes and rotations for all cubes at all time points I want to render.

I had a rough draft of the code in [`demo.c`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/gamebreak/demo.c) which I used as skeleton
of the actual [`demo.asm`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/gamebreak/demo.asm). The latter of course grew step by step.

### Step 1: Texture

![image](https://user-images.githubusercontent.com/6524684/102723646-97792e00-4309-11eb-9641-cc305bd0f724.png)

The first step was to create a texture and get it into EMU 1.0. I tried posterizing the original 16x16 texture in Paint.NET but that looked awful:

![image](https://user-images.githubusercontent.com/6524684/102723533-e4103980-4308-11eb-909e-b3437124b107.png)

I tried to achieve better dithering by hand, but you just end up with something that looks more like bedrock than redstone, and 3D renderings
of high-frequency textures on a 64x64 screen with nearest neighbor lookup were guaranteed to look terrible.

I know nothing about pixel art, but using a few posterized gradients as a color palette (you really only have two different shades of gray) I got
to work. After a bunch of manual pixel editing (and more failed attempts that were too busy), I arrived at this, which I was happy enough with:

![image](https://user-images.githubusercontent.com/6524684/102723605-64cf3580-4309-11eb-8c2f-80e43fa20ee5.png)

The tape has infinite storage in principle, and performance is somewhat free at these loop sizes. But making the tape random address accessable
would mean tripling the amount of storage required (you need a label to jump to, an instruction for storing data, then a jump backwards) compared to
copying data to the *random access memory* that comes with the GAMEBREAK. After briefly tripping over the requirement that the `io r00, MEM_WRITE, x`
instruction of course needs a register for `x` and not an immediate (easily solved for the texture because there are very few distinct pixel values,
so I just did `mov r32, 32` beforehand so I could do `io r00, MEM_WRITE, r32` to store a `32`), I managed to copy the texture data to RAM and render
it from there (divide the GPU coordinates by four to get the texture coordinates) as a first successful test.

The conversion from texture to instructions on tape is in [`img2mem.py`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/gamebreak/img2mem.py)

### Step 2: Matrix math

The next step was the main challenge: Implementing matrix-vector-multiplication. With 6 bit fixed point numbers. Where matrix entries could be arbitrarily
large or small. Oh boy...

My plan was to do multi-word arithmetic: Space coordinates x and y are 6.0, matrix entries are 6.6, accumulator for u and v is 12.6 (see `demo.c`).
Which essentially worked, except I more or less just guessed how to get the sign handling and overflow checks right. It seems that I didn't get them right.

But that was only one of several problems. Another one was (predictably) the limited range of the matrix entries: If your ideal transformation matrix says
"Take the space x and multiply it with 100" while your matrix entries (in 6.6 format) can only range from about -32 to 32, you run into trouble. This
happens for example when one of the faces is almost orthogonal to the viewer (i.e. the face is viewed from the side): Incrementing your screen x coordinate
by 1 means that you would have to advance many pixels in the u coordinate. One mitigation would be to avoid these kinds of extreme angles.

Nevertheless, I went ahead and threw some test matrices at my implementation. It was encouraging to see that if I used a plain scaling matrix (e.g.
the matrix representing "divide x by 4 to get u and y by 4 to get v" neatly rendered the redstone texture into the entire window.

For what it's worth, doing a memcpy from tape to RAM is actually best accomplished by setting all registers to their own number and then using
register numbers as immediates (i.e. the same thing explained above for the texture, extended to all registers). This allows you to write one
word of RAM per instruction.

### Step 3: More matrix math

Then came the part of building the actual matrices that I wanted to use for the demo. My initial idea was to just have a bunch of cubes rotating in place,
so I slowly built up to that: Start with a single face (= 1 plane = 1 matrix) without any rotations. Then add rotation about one axis to that. Then add
multiple (euler) rotations. Then add multiple faces. I really didn't look forward to this part, and as with anything involving 3D transforms it starts
with "I can intuit about this and don't need to build it up rigorously", followed by an hour of trying to figure out why *this matrix that should do
what you want just does something completely different*, followed by admitting defeat and very slowly and incrementally building up from elementary
operations. You can find the resulting code in [`trafos2mem.py`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/gamebreak/trafos2mem.py).

Here is a screenshot from when I first got three separate cube faces to render in a somewhat acceptable manner:

![image](https://user-images.githubusercontent.com/6524684/102723331-5718b080-4307-11eb-8935-b13df5cc8dad.png)

The gaps between the cube faces are the result of imprecise fixed point math. At some point I decided it would be best to give the cube faces a border
(kind of like when you look at a block in Minecraft) which would hide this effect a bit (instead of allowing you to see the background through the edges
of the cube). It also meant that I could move the cube faces inwards a bit to offset this imprecision by causing the faces to intersect:

![image](https://user-images.githubusercontent.com/6524684/102723729-56cde480-430a-11eb-8945-f1f24001f022.png)

### Step ???: The fixed point strikes back

Given the 12.6 accumulator format I used for u and v, I imagined I would be checking `uHi == 0` and `uMid < 16` and that would be all I need. However, I
just couldn't get my carries in the multiplication code right, so the `uHi == 0` check was wrong more often than not. I resorted to essentially disabling
this check, which of course means that the texture now actually repeats: *Every* part of the plane where `uMid < 16` and `vMid < 16` would end up with
the cube texture. That's not 100% bad (it makes the demo quite busy with only very few hardcoded planes), but it's not exactly the kind of success I
had imagined. A bunch of trial and error led nowhere, so I had to work around or accept this in the demo.

I also elected to have the `m02` and `m12` entries be 12.0 fixed point instead of 6.6, as the fractional precision there made no meaningful difference
(especially since my carries didn't work correctly) and I really needed those to cover more than -32 to 32.

### Step 4: Write demo

I'm not going to bore you with the creative process here. However, I did try to specifically work around some of the above problems. For example, I wanted
to prevent having "extreme" angles for the cube faces. A good starting point there was to have one corner of the cube point at the observer (and then have
mild rotations around this point). This is what you can find in the `trafos2mem.py`.

Originally I wanted to have a bunch of cubes rotating in place. But it struck me that I could get a nicer demo by having them move instead. Using a bunch
of interpolation for centers, rotations and size, I could fake 3D movement. It of course exacerbated the repeated cubes, but the effect was actually
rather cool. If ~~life gives you lemons~~ you have broken fixed point math, design your demo around it! 

There's also another trick in there: I only have two cubes (i.e. 6 planes/faces) that are rendered, and the demo is 32 frames. But the cube movement from "spawn"
to going offscreeen takes 64 frames... That's achieved by transparently having the cubes "switch places": Cube 1 moves from the middle to the bottom left,
and cube 2 spawns in and moves to the middle. As long as the last position of one is equal to the first position of the other, it looks seamless.
I spent a bunch more time tuning everything to get the cubes rather clean and as few distracting "fake"/"bad carry" cubes interfering with the visuals as
reasonably possible.

Why is the demo 32 frames? Because I just used `MEM_ADDR_HI = 32 + frameNum`. Yes, I could fit days worth of animations into the ram if I wanted to, but
I had enough problems to deal with and no time to spare.

### Step 5?: Background

At this point it was a cool tech demo but otherwise pretty bare-bones. Cubes in front of a uniform background is cool but it needed something more.

I had some ideas for a background, like occasional comets, or a tron grid where signals move along the grid lines. Making a straight grid is easy enough,
but it looks terrible behind FaNcY 3D gRaPhIcS. Slanting the grid also didn't give me what I want, so I thought about giving the whole thing perspective by
having the "vertical" lines of the grid have a point of convergence. "Just multiply with the other coordinate, right?" Well, what I got was not what I
was looking for, but it had interesting structure at least:

![image](https://cdn.discordapp.com/attachments/657666959235612672/789844554403545088/unknown.png)

With some zooming in (changing `fmu` precision), flipping some coordinates and an impromptu `and 20` (for some amount of chaos) on the multiplication
result I converged on the background you see in the final demo. For reference, here is what using `r03` directly instead of the `and 20` + `cmp`
stuff would look like:

![image](https://cdn.discordapp.com/attachments/657666959235612672/789844060709978131/unknown.png)

---

All in all, this was an exciting project, and while the result is not fully what I originally had in mind, it turned out better than I hoped!
Really looking forward to next year's emu challenge and demo party. Maybe it's about time to implement a game? :)

![image](https://user-images.githubusercontent.com/6524684/102724404-9945f000-430f-11eb-8404-1e0ff2a9c631.png)
