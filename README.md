Sceptre
=======

Sceptre language is a language that could be easily compiled down to JavaScript

The compiler is written in C++ and we aim for the speed and universality. Sceptre is unlike CoffeeScript or other thousands of JavaScript compilers out there. Sceptre is a designed language that aims to feel as close to JavaScript as possible. It provides meta-structure to your JavaScript program, without tampering your JavaScript program.

Right now it's about 20% done. Here is some implementation details:

Lexical Analyzer: Hand-coded DFA
<br>
Syntactical Parser: LL(1) grammar, recursive descent parsing

<br>
Beyond simple parsing, Sceptre will attempt to bring two additional functionalities to JavaScript: 

1. Concurrency framework (JavaScript is asynchronous by nature but without a concurrency framework, it can be difficult to maintain); The main choice for Sceptre would be an Actor model instead of lock-based threading; 
2. Functional programming. JavaScript is functional by nature but never promotes the idea of immutability. Sceptre will try to bring immutability to JavaScript as well.
