from pneu_arm import PneuArm
import numpy as np
import matplotlib.pyplot as plt
import seaborn as sns

import time

sns.set_theme(
    context="talk",
    style="whitegrid",
    palette="deep",
    font="sans-serif",
    font_scale=1.5,
    color_codes=True,
    rc=None,
)


def main():
    arm = PneuArm()

    freq = 100

    steps = 10_000

    loggings = np.zeros((steps, 9))
    for i in range(steps):
        start_time = time.time()

        outputStates = arm.CommGetData()
        # print(outputStates)
        # loggings
        loggings[i] = outputStates.squeeze()

        inputCommands = np.zeros(4)
        inputCommands[0] = 2 + np.sin(i / 10)
        inputCommands[1] = 2 - np.sin(i / 10)

        # send commands to target
        arm.CommSendData(inputCommands)

        end_time = time.time()
        elapsed_time = end_time - start_time
        if elapsed_time < 1 / freq:

            time.sleep(1 / freq - elapsed_time)

    # plot
    fig, ax = plt.subplots(3, 3, figsize=(15, 15))

    for i in range(9):
        ax[i // 3, i % 3].plot(loggings[:, i])
        ax[i // 3, i % 3].set_title(f"Joint {i + 1}")

    plt.show()


if __name__ == "__main__":
    main()
