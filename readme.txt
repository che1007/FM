編譯: make

執行:
./FM_Q36111150  檔名.nodes 檔名.nets 檔名.out
ex :
./FM_Q36111150  adaptec1.nodes adaptec1.nets adaptec1.out $times

!!!!!!
$times代表執行FM的次數，加在整串指令後面(可以不加，代表default，為1次)。
2代表執行2次，依此類推。我有在excel裡註記執行次數。

會得到adaptec1.out的輸出檔

FM_checker:(FM_checker_run.sh)
./FM_checker adaptec1.nodes adaptec1.nets ./adaptec1.out
./FM_checker adaptec2.nodes adaptec2.nets ./adaptec2.out
./FM_checker adaptec3.nodes adaptec3.nets ./adaptec3.out
./FM_checker superblue1.nodes superblue1.nets ./superblue1.out
./FM_checker superblue2.nodes superblue2.nets ./superblue2.out







