n = 0


def initialize():
    return 'initialize'

def computeTile(x, y, array):
    global n
    n += 1
    spend_some_time()
    return 'computeTile-' + str(n) + '-' + str(x) + ',' + str(y)

def dispose():
    return 'dispose'


def spend_some_time():
    l = list(range(10000))
    for i in range(10000):
        l.reverse()




