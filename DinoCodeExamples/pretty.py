# coding=utf-8

s = open("Std.dino").read()
s = s.replace(' :=', ' ≡').replace(' !=', ' ≠').replace(' <=', ' ≤').replace(' >=', ' ≥') #.replace(' / ', ' ÷ ').replace(' * ', ' × ').replace(' /= ', ' ÷= ').replace(' *= ', ' ×= ')
f = open("Std.dino", 'w')
f.write(s)
f.close()

