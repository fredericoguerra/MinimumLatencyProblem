import os
import subprocess

dir_path = './instances'

for filename in os.listdir(dir_path):
    if filename in ['dantzig42.tsp','swiss42.tsp','att48.tsp','gr48.tsp','hk48.tsp','eil51.tsp',
    'berlin52.tsp','brazil58.tsp','st70.tsp','eil76.tsp','pr76.tsp','pr76r.tsp','gr96.tsp', 'rat99.tsp',
    'kroA100.tsp', 'kroB100.tsp', 'kroC100.tsp', 'kroD100.tsp' , 'kroE100.tsp', 'rd100.tsp', 'eil101.tsp', 'lin105.tsp', 'pr107.tsp']:
        print(filename)
        for i in range(0,20):
            print(i)
            proc = subprocess.Popen(['./mlp',"{}/{}".format(dir_path, filename)])
            proc.wait()