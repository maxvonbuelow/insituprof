import sys
import os
from builtins import bool
import matplotlib.pyplot as plt
import numpy as np
import numpy.random as nprnd
import time

buffer_map = {
    "4096": "vertices",
    "4097": "tetrahedra",
    "4098": "level nodes",
    "4099": "primitive offsets",
    "4100": "node aabbs",
    "4101": "children offsets",
    "4102": "boundary flags per cell",
    "4103": "primitive indices",
    "4104": "scalars per vertex",
    "4105": "transfer function",
    "4106": "samples per pixel",
    "4107": "rays",
    "4108": "ray intervals",
    "12345": "BRANCHING",
    "12346": "REQS"
}


def visualize_buffer(buffer, width, height, dd):
    margin=50
    dpi = 100.
    width = int(width)
    height = int(height)
    figsize = ((width + 2 * margin) / dpi, (height + 2 * margin) / dpi)
    left = margin / dpi / figsize[0]
    bottom = margin / dpi / figsize[1]

    x = np.arange(width)
    y = np.arange(height)
    x, y = np.meshgrid(x, y)

    # create scatter plot
    plt.rcParams["figure.figsize"] = figsize
    plt.rcParams["figure.dpi"] = dpi
    plt.rcParams["figure.subplot.left"] = left
    plt.rcParams["figure.subplot.bottom"] = bottom
    plt.rcParams["figure.subplot.right"] = 1. - left
    plt.rcParams["figure.subplot.top"] = 1. - bottom

    plt.xlim(0, width)
    plt.ylim(0, height)
    vm = 1
    if (buffer[0] == 12346):
        vm = 32768
    plt.scatter(x, y, c=buffer[1][x, y], vmin=0, vmax=vm, cmap="afmhot")
    plt.gca().invert_yaxis()

    plt.savefig(dd + "/" + str(buffer[0]) + "_" + buffer_map[str(buffer[0])] +  ".png")


def do_cache(dd):
    with open(dd + "/countedCacheMiss.txt") as f:
        lines = f.read().splitlines()
    width, height = lines[0].split(":")

    buffer_list = []

    for line in lines[1:]:
        split = line.split(":")
        if (split[0] == "newBuffer"):
            buffer_id = int(split[1])
            pixel_array = np.zeros((int(width), int(height)))
            new_buffer_tuple = (buffer_id, pixel_array)
            buffer_list.append(new_buffer_tuple)
        else:
            buffer_list[-1][1][int(split[0])][int(split[1])] = float(split[2])

    for buffer_tuple in buffer_list:
        visualize_buffer(buffer_tuple, width, height, dd)


if sys.argv[1] == "1":
    do_cache("./__fin_514/data_cache_0")
    do_cache("./__fin_514/data_cache_1")
    do_cache("./__fin_514/data_cache_2")
    do_cache("./__fin_514/data_cache_sr_0")
    do_cache("./__fin_514/data_cache_sr_1")
    do_cache("./__fin_514/data_cache_sr_2")

    
