import jpy

jpy.create_jvm(options=['-Xmx512M'], debug=True)

S = jpy.get_class('java.lang.String')
s = S('Hello JPy!')

print('S =', S)
print('str(S) =', str(S))
print('s =', s)
print('str(s) =', str(s))

s = s.substring(0, 5)

print('s =', s)
print('str(s) =', str(s))

jpy.destroy_jvm()
