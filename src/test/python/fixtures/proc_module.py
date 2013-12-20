n = 0

def initialize():
    global n
    n += 1
    return 'initialize-' + str(n)

def computeTile(x, y, array):
    global n
    n += 1
    return 'computeTile-' + str(n) + '-' + str(x) + ',' + str(y)

def dispose():
    global n
    n += 1
    return 'dispose-' + str(n)




