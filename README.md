#### Flowers The Remix ############################################

<p>Taking the previous proof of concept and flower game w/ two myos and turning it into a basic demo that is not a game. </p>
<p>Two people need to perform matching gestures to open each of the two flowers, or open both of them together.</p>

## KNOWN ISSUES ####################################
<p>The Myo API doesn't include a local name, or UUID that is passed up ATM, so the fudge is counting connected myos in a list. If one myo is connected it is considered 'Player1' but that could be any myo you have.</p>

#### Resources ############################################
<p>Python Myo Library is from over here: https://github.com/NiklasRosenstein/myo-python</p>