# EMU 1.0

The EMU 1.0 digitisation project, lead in 2005 by Tahlia E., aimed to immortalise the almost forgotten EMU 1.0 architecture's program collection.
They did not fully succeed, and all their work fell to obscurity, but a naughty elf got their hands on Tahlia's manual and a few of her programs.
The current issue is they can't find a working EMU 1.0 machine to actually run the programs. Figure out how the computer worked and get the code running!

**Author:** trupples  
**Category:** Emulation  
**Files (original):** [mandelbrot.rom](https://drive.google.com/file/d/1IdSazwFbhm13Ln6PlYItDuM5TZI9Y4Z3/view?usp=sharing),
[Thalias_EMU_1.0_Manual.pdf](https://drive.google.com/file/d/13afjSCTDuqDgCHt_3MbREHcj6sYCJa_G/view?usp=sharing)  
**Files (cached):** [mandelbrot.rom](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/emu-1.0/files/mandelflag.rom),
[Thalias_EMU_1.0_Manual.pdf](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/emu-1.0/files/Tahlias_EMU_1.0_Manual.pdf)  

![image](https://user-images.githubusercontent.com/6524684/102719337-ce414b00-42ed-11eb-8ee7-0e97948d56c3.png)

---

This was the task that all emulation challenges this year built on. Build targets include the emulator itself (`emu10`, implements GAMEBREAK
extension but not Talkative), a rom -> asm disassembler (`emu10dasm`) and an asm -> rom assembler (`emu10asm`). Files here are as they were
left at the end of the challenge: The emulator and assembler only implement the GAMEBREAK extension but not Talkative, and the disassembler
(which I used for solving Talkative) doesn't care. 

Run `make` to build the emu, disassembler and assembler, or `make fast` to build the emu with optimizations enabled.

Some specific experiences and lessons learned:

- I felt good about our approach from last year (https://github.com/redstoneblockchain/X-MAS-2019/tree/master/emu2.0) so I used the same structure.
  Particularly the large opcode switch statement into individual functions is very clean, readable and flexible.

- `using Word = std::uint8_t` looks to have been the right choice. Introducing the 6 bit abstraction as a class was an option, but based on the
  annoyances we had last year with the double registers, it seemed easier to stick with a fundamental type and just handle the truncation in
  reads and writes directly.
  
- Use asserts. I don't think they revealed any major bug (mainly just "you forgot to loop the tape"), but even just knowing that certain cases
  are not occuring greatly helps in tracking down problems.
  
- I was not the only one to be confused about how the comparison statements were meant to work (and when/how/where the underlying flags would be
  set). trupples clarified that we can ignore those flags and can just implement the comparison operations with the obvious intuitive results.
  This was something I wanted to be 100% sure about: Last year, we unknowingly assumed a different carry flag convention than the author,
  leading to hours upon hours of unfruitful debugging.
  
- I had no choice but build a disassembler (like we did last year) to aid in debugging. This time around it was *significantly* cleaner though,
  primarily due to working with a Harvard architecture. I also implemented a rudimentary GDB-like command line debugger. Not sure if I actually
  found any bugs with it, but it did help with the initial exploration of the mandelflag rom.

- A key thing to get right was the handling of signed and unsigned (i.e. arithmetic vs logical) shifts. Specifically, you had to get the sign
  extension from 6 bit to 8 bit and from 12 bit to 16 bit right, or the shifts would not produce correct results. This is all contained in `SixBit.h`.

- After the initial implementation, I couldn't get it to output the right thing. After working through the manual two more times and fixing a handful
  of bugs, I realized that my serial output was `std::cout << CHARSET[rs];` instead of `std::cout << CHARSET[read(rs)];`... But that thorough search
  for bugs beforehand meant there were no further ones to squash. :)
  
- In general, I kind of half-assed the device IO. I didn't want to spend time implementing stuff that was not necessary for the flag (I missed the
  podium on this challenge by 10 minutes, fwiw), so that function is somewhat of a mess. And it still doesn't implement networking at all...

- Having the SDL stuff (mostly) figured out from last year (including a pretty much complete `Screen.h`) helped a ton. Thanks Kris!

- The disassembler and assembler support a `mov` alias for `add rXX, r00, YY` and `add rXX, rYY, 0`. This turned out to be a very useful and convenient
  idea.
