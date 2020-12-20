# Talkative

Now that you have an EMU 1.0 emulator, go make a network card and come chat with us!  
P.S. you don't need to implement the server card to get the flag

**Author:** trupples  
**Target:** nc challs.xmas.htsp.ro 5100  
**Category:** Emulation  
**Files (original):** https://drive.google.com/drive/folders/1xl1_mjtA7Sk4XT4eEZzom0I-4qM6QcAu?usp=sharing  
**Files (cached):** https://github.com/redstoneblockchain/X-MAS-2020/tree/main/talkative/files/  

![image](https://user-images.githubusercontent.com/6524684/102720421-896ce280-42f4-11eb-98f8-b2dd7e1badf2.png)

---

Right after finishing the emulator (and barely missing third place there), I went to look at the second emu challenge.
It was still unsolved, and after multiple second places I set my mind on getting first blood on this one. And so I did!

After reading through the provided manual, I directly went to disassemble the server rom that held the secret.
Therefore, none of the network capacities are actually implemented in any of the emulation code (not even the related
named constants) - it wasn't necessary for obtaining the flag.

[`e2c.txt`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/talkative/e2c.txt) contains my annotated version
of the disassembly. I also added the full disassembly as
[`talkative-server-redacted.asm`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/talkative/talkative-server-redacted.asm)
for reading convenience.

---

To summarize, the server polls all clients until it finds a client with 40+ words in the buffer. It then interprets the first
four words as a four character username and the rest as the message, both of which are dumped to memory with `: ` between them.
The server then checks whether the username and message match a particular "backdoor", in which case it jumps to some code
that is omitted from the rom (which presumably gives you the flag). Otherwise, it cycles through all remaining connections
and sends the full message to all available clients. Rinse and repeat.

The secret message is "hidden" in that the backdoor check does not check the registers in order. I took those lines of assembly
to Notepad++ and after some regex + line sorting + more regex I had the words of the backdoor message. Converted with the
charset (and with `: ` added), they are equivalent to sending this chat message to the server:

`YMAS: I MIGHT BE BETTER THAN X-MAS LOLOLOL`

Finally, [`py.py`](https://github.com/redstoneblockchain/X-MAS-2020/tree/main/talkative/py.py) sends those words to the server
and decodes the response for me:

`X-MAS{Y0U-$UR3-AS-H#LL-4REN#T!!1!!}`
