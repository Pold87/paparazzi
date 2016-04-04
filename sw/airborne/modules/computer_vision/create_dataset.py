import numpy as np
import pandas as pd
import os

def main():
    histograms = pd.read_csv("mat_train_hists.csv", header=None).values
    targets = pd.read_csv("board_train_pos.csv")
    targets.drop(['id', 'matches'], inplace=True, axis=1)

    msk_too_big         = targets > 1500
    msk_too_small = targets < 0

    targets[msk_too_big] = 1500
    targets[msk_too_small] = 0
    
    targets =    targets.values

    print("min is", targets.min())
    print("max is", targets.max())    

        
    targets =   targets - targets.min()
    targets =   targets / 1500
    
    N, D        = histograms.shape
    header =     np.array([N, D, 2])

    
    datafile_fp = open("dataset.data", 'ab')
    np.savetxt(datafile_fp, header,     newline = ' ', fmt=  "%d")
    datafile_fp.write("\n")

    for h, t in zip(histograms, targets):
        np.savetxt(datafile_fp, h, newline = ' ', fmt='%d')
        datafile_fp.write("\n")
        np.savetxt(datafile_fp, t, newline = ' ', fmt='%f')
        datafile_fp.write("\n")

    datafile_fp.close()


if __name__ == "__main__":
    main()
