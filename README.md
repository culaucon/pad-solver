# PAD Solver

This repository contains the automatic orbs-matching feature for Puzzle and Dragons.
Currently the code has some basic heuristics for different teams, like maximizing combos count for combo Gods, TPA for Green Zhuge Liang, Athena... Other heuristics and be easily added.

# Requirements

1. Puzzle and Dragons game installed on an Android phone.
2. Linux machine with root access and adb shell for Android installed (no need root access for Android).
3. USB cable.

# Flow

1. Use a USB cable to connect the computer with the Android phone having a puzzle board on screen.
2. Sudo run the compiled binary:

  2.1 The binary will first take a screenshot of the phone and display the analyzed board content. Double check if this is the correct board as there can be issues with identifying enhanced orbs (refer to Issues).
  
  2.2 Depending on the team, the orb path yielding the best heuristic will be computed. The final board after applying the best path will be displayed. The board strings that are Dawnglare-friendly will also be outputed for easy double check with http://pad.dawnglare.com/
  
  2.3 Press Enter to dispatch adb events to simulate swipe events on the Android for orb matching.

3. Witness all the crazy cascades and wait for the next bosses.

# Issues

1. For now, the code can only identifies Red, Green, Blue, Dark, Light and Heart orb types. If Jammer or Poison orbs are present, the board content can be manually edited accordingly.
2. Taking screenshot of the enhanced orbs at different time yields different color content. Specifically, enhanced green orb can sometimes be identified as blue orb (refer to "wrong.png" and see that the 2 enhanced green orbs look awfully blue).
