# The Dino Programming Language
**_Built from the ground up to look pretty._**

Dino is a C-like language with a focus on being aesthetically pleasing.

See the [language specification](https://docs.google.com/document/d/11jGM8cC0KQR9h4z7kOGGzQrC-q2TgwGZS2JI9Ti4QCI) for more details.

### No Semicolons

```
foo.Foo()
a() | b()  // the line break operator is '|'.
```

The most important thing in a language, really.

### Unicode Operators

```
if b ≠ 0:
    a ≡ b
```

The IDE will make entering them convenient, I promise!

### And a bunch of Syntatic Sugar

```
// unless - equivalent to "if not":
unless a: // Use ':' rather than braces if a block only has one statement
    return 10

// postfix conditionals
break if a = 10

// python-style comparisons
if 10 < a ≤ 20 { /* ... */ }

// Shorthand variable declarations
int a ≡ 1
var a ≡ 1
a :≡ 1

// easy type checks and conversions
if a is int {
	i :≡ a as int
	// Do stuff with integers...
}
```

## Example program

An interpreter for the esoteric language [Brainfuck](https://en.wikipedia.org/wiki/Brainfuck). 

This example can currently be fully built by the compiler.

```
import "std"

namespace BrainF {
    
    char[30000] tape

    void Interpret(string input) {
        int ptr ≡ 0
        for i :≡ 0 | i < input.Size | i++ {
			switch input.Get(i) {
				case '>': ptr++
				case '<': ptr--
				case '+': tape[ptr]++
            	case '-': tape[ptr]--
				case '.': Std.PrintC(tape[ptr])
				case ',': tape[ptr] ≡ Std.GetChar()
				case ']': if tape[ptr] ≠ '\0':
					for loop :≡ 1 | loop > 0
						switch input.Get(i--) {
							case '[': loop--
							case ']': loop++
						}
			}
        }
    }

    void Main() {
        // Hello World!
        Interpret("++++++++[>++++[>++>+++>+++>+<<<<-]>+>+>->>+[<]<-]>>.>---.+++++++..+++.>>.<-.<.+++.------.--------.>>+.>++.")
    }
}
```

## Implementation Status

### What's missing in the Lexer?

* Strings with escaped characters (eg. "\n")

### What's missing in the Parser?

* Type switches

* Groups

* Generic types

* Vararg functions

### What's missing in the Semantic Analyzer?

Everything that's missing in the parser plus:

* Blank identifier

* Generics

* break/continue (I'm pretty sure they're broken atm)

### What's missing in the Code Generator?

Everything that's missing in the parser plus:

* Closures (aka function literals)

* Switches with non-constant/enumerable values

* the "is" operator for things known at runtime

* array bounds checks

* proper debug information

### What's missing in the Specification?

* A proper explanation for how literals are handled

* A proper explanation for namespaces

* A proper explanation for function calls

Some ideas ordered by the chance of them actually ending up as part of the final design:

1. Implicit conversions for literal types

2. Initializers

3. Better syntax for try/catch

4. "each" loop (aka "foreach") + range operator

5. Type aliases

6. Type extensions

7. Operator overloading

8. Default function argument values

### More things to do

* implements a runtime for the language and link it in when compiling

* implement some sort of basic standard library

* some useful warning (unused variables, etc.)