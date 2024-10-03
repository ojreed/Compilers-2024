Owen Reed Compilers HW2 MS2

The main steps I took to continue my Assignment 2 were as follows:


1) Refactor how I implemented intrinsic functions. I was doing the actuall calling of the intrinsic inside the enviorments in a fn_call function, 
    but I dont want to return an evaluated value for intrinsics and some other things for non-intrinsic. Thus I brought a lot of the work back
    to the interpreter. This let me get the function from the enviorment and either evaluated it trivially or execute the AST nodes associated with its body
2) My basic enviorment management was not correct to start because when calling a function from inside another function it would inherit the enviorment of the callers body when its
    should inherit the enviorment of its definer. I had to do some subtle moving around of envs to get this to work
3) I then went to work on arrays. They were pretty easy. The tricky part was finding the way to define an array_value class but that just involved taking some time to read through
    files that I had not read in as much depth. 
4) at the very end I implemented reference counting