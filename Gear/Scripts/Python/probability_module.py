import random
# from random import randint
'''
use a seed value in your main calling functions
if you want some reproducability in your randomness
'''

def bool_from_probability(value):
    if value == 1.0:
        return True
    if value == 0.0:
        return False
    
    x = random.random()
    if x <= value:
        return True
    else:
        return False



def probability_trigger(probability):
    '''
    input:      probability value from 0.0 to 1.0 (including)
    return:     boolean, or None by invalud input.
    '''

    #must be an int between 0, 1.  else print error and halt.    
    if probability.__class__ == float:
        if probability >= 0.00000 and probability <= 1.00000:
            return bool_from_probability(probability)
            
    # reaching here means the input value was out of range.    
    print('pass a float between (and including) 0.0 and 1.0')
    return None
    
    
'''
# test
bool_list = [probability_trigger(0.8) for i in range(200)]
bool_list_true = [i for i in bool_list if i == True]
print(len(bool_list_true)/len(bool_list) )
'''

