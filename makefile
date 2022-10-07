all: output
		./output

output: bs_AES32_share.o bs_w32_AES.o main.o common.o prg3.o share.o present_crv.o present_htable_PRG.o present_shares_prg.o present.o aes_htable_PRG.o aes_rp.o aes_shares_prg.o aes.o
		gcc bs_AES32_share.o bs_w32_AES.o main.o common.o prg3.o share.o present_crv.o present_htable_PRG.o present_shares_prg.o present.o aes_htable_PRG.o aes_rp.o aes_shares_prg.o aes.o -o output

main.o: main.c
		gcc -c main.c

aes.o: AES/aes.c
		gcc -c AES/aes.c

aes_rp.o: AES/aes_rp.c
		gcc -c AES/aes_rp.c

aes_htable_PRG.o: AES/aes_htable_PRG.c
		gcc -c AES/aes_htable_PRG.c

aes_shares_prg.o: AES/aes_shares_prg.c
		gcc -c AES/aes_shares_prg.c

bs_AES8_share.o: AES/bs_AES8_share.c
		gcc -c AES/bs_AES8_share.c

bs_w_AES8.o: AES/bs_w_AES8.c
		gcc -c AES/bs_w_AES8.c

bs_AES32_share.o: AES/bs_AES32_share.c
		gcc -c AES/bs_AES32_share.c

bs_w32_AES.o: AES/bs_w32_AES.c
		gcc -c AES/bs_w32_AES.c


common.o: Util/common.c
		gcc -c Util/common.c

share.o: Util/share.c
		gcc -c Util/share.c

prg3.o: Util/prg3.c
		gcc -c Util/prg3.c

present_crv.o: PRESENT/present_crv.c
		gcc -c PRESENT/present_crv.c

present_htable_PRG.o: PRESENT/present_htable_PRG.c
		gcc -c PRESENT/present_htable_PRG.c		

present_shares_prg.o: PRESENT/present_shares_prg.c
		gcc -c PRESENT/present_shares_prg.c

present.o: PRESENT/present.c
		gcc -c PRESENT/present.c
clean_windows:
		del /F /Q  output.exe *.o
clean_linux:
		rm *.o output
