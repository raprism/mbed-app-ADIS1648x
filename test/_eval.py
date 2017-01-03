# coding: utf-8
import pandas as pd

class Options:
    "argparse dummy"
    interactive = False

if __name__=='__main__':

    options = Options()

    accel = pd.read_csv('out.csv', sep=';',
                        names=['ACCL_' + c
                               for c in ['X', 'Y', 'Z']],
                        index_col=False)

    print(accel.describe())

    ax = accel.plot()

    if options.interactive:
        # alternatively could be tested, if called in IPython
        ax.figure.show()
    else:
        ax.figure.savefig('out.png')
