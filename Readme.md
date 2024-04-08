# CYCU OurC : A C-like interpreter

## 心得&背景 

1. 分享一下當初完成PL的過程，本人學號107開頭 (不怕死的就抄吧)

2. 絕對不用要 vector.back() 來取得vector的最後一個元素，我因為這個差點放棄Proj4. 的後半部，後來猜到不能用才全過...

3. 如果有發現任何Bug，歡迎聯繫我~

## OurC開發日誌

### Project2 
Proj. 2 is the first part of OurC project (there are three parts). 
For Proj. 2 
you are to implement a syntax checker and a pretty printer that supports system-supported functions.


- 4/12 syntax flow chart 
- 4/13~14 syntax coding on definition  **end with 1191 lines** 
- 4/15 coding on statement **end with 2435 lines** 
- 4/16 coding on supported functions **end with 2847 lines** 
- 4/17 fixing grammer **end with 2784 lines** 
- 4/18 coding on sematic check **end with 3000 lines** 
    - solving erorr + peektoken+ encounter // comment
- 4/19 卡隱藏 優化後**end with 2229 lines** 
    1. cout << A() ;
    2. pretty print of if else while without { }
    3. statement should include supported function 
- 4/21 still finding some error... 
    1. escape sequence \
    2. true & false is constant 
- 4/23 guessing prob.10 the wrong part by inf loop
    1. no EOF
    2. no pretty print 
    3. no function def
    4. has compound statement 
    5. has supported 
    6. still need to solve but project 3 first 
    7. grammerly buf fixxing a[i] < a[j] ; **FIXED**
    7. supported functions // need to fix if without {}

### Project3 

- 4/26 document + decalrator 
    1. decalrator solved 

- 4/27 expression + cout **end with 2520 lines** 

- 4/28 multiple assign simultanously PAL no pointer 
    - nested conditional exp

- 4/29 
    1. loop 
    2. compound statement of not executing 
    3. not executing with eval 
    4. float to string notation 
    5. nested loop 

- 4/30
    1. nested loop with outer positioning 
    2. compound statement of not executing with while loop 
 

### Project4 

- 5/1 
    1. function call

- 5/2 
    1. function call
    2. end at  int a [10] is pass by ref

- 5/3
    1. function call
    2. expression value location 
    3. should if roll back ?
    4. conditional exp should check first 
    5. looprecover should use stack 
    6. pass array element 
    7. nested function call

- 5/4
    1. nested function call passing parameter ..
    2. function scope + none_var scope OK

- 5/5 code review day  
    - to do 
    1. recursive function  done !
    2. middle return done!

- 5/6 coding cleansing 
    1. recursive function  done !
    2. middle return done!

- 5/7
    1. index init
    2. end to end pass value 
