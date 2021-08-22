sort.exe arm7.map > arm7_sort.map
arm7_objdump.exe -a arm7.elf > arm7_objdump_a.txt
arm7_objdump.exe -f arm7.elf > arm7_objdump_f.txt
arm7_objdump.exe -p arm7.elf > arm7_objdump_p.txt
arm7_objdump.exe -h arm7.elf > arm7_objdump_h.txt
arm7_objdump.exe -x arm7.elf > arm7_objdump_x.txt
arm7_objdump.exe -s arm7.elf > arm7_objdump_s.txt
arm7_objdump.exe -S arm7.elf > arm7_objdump_src.txt
