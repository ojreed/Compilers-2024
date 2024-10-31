I had a beyond busy week so I didnt finish two error checking things.

The big one is that we do not ban re-declaration of functions. The way I saved code by reusing a chunk of declaration makes this 
complicated to fix and now that it is horribly late I do not have the bandwidth to fix this. What I would do is create some aux data 
structure to track declatrations and definitions of each function to throw an error when they are re-declared. Id also need to not have my function
definition function call declaration directly because this makes it hard to differentiate the two distinct processes for error checking.

The other issue is that I am having trouble with function argument/parameter assignment type checking. Idk what the issue is but I simply will not get you an answer right now/

Nothing I did was incredibly interesting but it kinda just all works. Some of it can should and will be refactored.