Owen Reed Compilers HW2 MS1

The main steps I took to modify my Assignment 1 were as follows:

1) add new content to my lexical analyser 
2) update parser to account for ALL productions assoicated with A2M1 and A2M2. I wanted to make sure I fully understood how everything should be arranged before making decisions about how to parse
3) Test parser/lexer with tree generation
4) Interpreter Functionality A2M2  
    a) update my enviorments to support child/parent relations and value ownership/access
    b) implement logic of control flow
    c) add logical operator short circuting by chaning old interpreter rules
    d) define intrinsic functions
    e) add parser suport for intrinsic functions and add placeholder code for non-intrinsic function calls
5) test/debug with public tests and gradescope secret test hints
 a) I was improperly checking if values were numeric by implicitly casting all as numeric. this meant my code would never actually see a non-numeric value until it already broke