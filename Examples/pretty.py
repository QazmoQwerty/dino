# coding=utf-8

s = open("Examples/Map.dino").read()
s = s.replace(' :=', ' ≡').replace(' !=', ' ≠').replace(' <=', ' ≤').replace(' >=', ' ≥') #.replace(' / ', ' ÷ ').replace(' * ', ' × ').replace(' /= ', ' ÷= ').replace(' *= ', ' ×= ')
f = open("Examples/Map.Dino", 'w')
f.write(s)
f.close()

