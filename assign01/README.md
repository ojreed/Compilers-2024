Owen Reed Compilers HW1

My loose approach to this assignment was as follows:

First I took the time to read through the project description and came up with a game plan and order of work for the different aspects. I decided to work with the following procedure: 
- First I would add all of the new lexer tokens and add support for identifying and creating proper tokens. This also involved coming up with a procedure to identify 2-part tokens
- Then I planned to add all of the needed productions and thier respective parse functions
- Then I would focus on making my parse tree format properly as an AST
- Next I felt it would make sence to add semantic checking
- Finally I would implement the ability for my program to actually execute the code represented 

The token creation was fairly straightforward. I simply added the requisit enums and then added case statments. The only tricky bit was differentiating multiple character tokens. This is broken into two problematic sub-cases. "Branching second characters" where after seeing our first char we to see the next character to differentiate if it is < or <=, and "needed second characters", for example if we ever read | we expect the next character to also be | and have a problem otherwise. 

The production implementation definitly took the longest of any part of this project. The main tricky bit was figuring out exactly the structure of my AST nodes and ensuring it encodes the proper infiormation. Starting by implementing the easier parse functions like parse_A and parse_STMT was helpful because it let me figure out how to work with nodes in an easy context before moving on to parse_L and parse_R. Parse_R ended up being an interesting process of always calling a parse_E and then potentally adding the comparison followed by another parse_E. I had a slightly hacky elif chain to differentiate each of the comparisons before funneling into a common end point for all operations that managed tree structure and the final parse_E. Parse_L was a little easier and basically copied the method of the T and E parsing without the need for L' because parse_L already steps down the recurse chain to R.

Getting it to work as an AST was fairly easy. I just had to make sure no nodes were created for NT tokens and ensure that the operator was always the parent in the parse R and parse L fucntions. 

Semantic checking was easy enough because there was only one rule we care about. This meant that for every statment I would either call a function to record a newly defined var OR find all var references in the statment and check to see they have all been defined. Recording a new var is easy because you just check for a VAR_DEF node and then store the name in a string. To search for all VAR_REFs in a statment I used a recusive function that returns a vector of found var names at each step.

Execute was also fairly easy as I simply looped through each statment and called a recusive exec_node function on it. These would return Values up the chain. Mostly this copied the procedure outline for an exec_node function defined in class. 