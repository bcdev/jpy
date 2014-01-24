n = 0

def initialize():
    return 'initialize'

def computeTile(x, y, array):
    global n
    n += 1
    return 'computeTile-' + str(n) + '-' + str(x) + ',' + str(y)

def dispose():
    return 'dispose'




