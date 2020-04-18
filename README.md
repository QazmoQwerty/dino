# The Dino Programming Language
**_Built from the ground up to look pretty._**

Dino is a C-like language with a focus on being aesthetically pleasing.

See the [language specification](https://docs.google.com/document/d/11jGM8cC0KQR9h4z7kOGGzQrC-q2TgwGZS2JI9Ti4QCI) for more details.

### No Semicolons

```python
foo.Foo()
a() | b()  # the line break operator is '|'.
```

The most important thing in a language, really.

### Unicode Operators

```python
if b ≠ 0 {
    a ≡ b
}
```

The IDE will make entering them convenient, I promise!

### And a bunch of Syntatic Sugar

```perl
# unless - equivalent to "if not":
unless a: # Use ':' rather than braces if a block only has one statement
    return 10
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
        for int i ≡ 0 | i < input.Size | i++ {
			switch input.Get(i) {
				case '>': ptr++
				case '<': ptr--
				case '+': tape[ptr]++
            	case '-': tape[ptr]--
				case '.': Std.PrintC(tape[ptr])
				case ',': tape[ptr] ≡ Std.GetChar()
				case ']': if tape[ptr] ≠ '\0':
					for int loop ≡ 1 | loop > 0 |:
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