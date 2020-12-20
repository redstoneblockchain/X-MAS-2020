
python img2mem.py > texture.asm
python trafos2mem.py > trafos.asm
cat demo.asm texture.asm trafos.asm > baked.asm
../emu10asm baked.asm > demo.rom
