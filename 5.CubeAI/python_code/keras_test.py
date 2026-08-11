try:
    import tensorflow as tf
except ImportError:
    tf = None
    print('TensorFlow is not installed. Please install tensorflow to run this script.')

import numpy as np
import matplotlib.pyplot as plt

if tf is None:
    raise SystemExit(1)

import os
script_dir = os.path.dirname(os.path.abspath(__file__))
model = tf.keras.models.load_model(os.path.join(script_dir, 'sine_calcu.h5'))
#model = tf.keras.models.load_model(os.path.join(script_dir, 'sine_calcu.tflite'))

true_values = [np.sin(i*np.pi/180) for i in range(0,360)]

predictions = model.predict([i for i in range(0,360)])

plt.plot(true_values)
plt.plot(predictions)
plt.show()

