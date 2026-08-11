# 训练 sin 拟合模型
# 关键：输入归一化到 [-1, 1]，TANH 不饱和才能学到负半周

import pandas as pd
import numpy as np
import tensorflow as tf

# 读取数据
data = pd.read_csv('./sin_values.csv', sep=',', header=0)
raw_x = data.iloc[:, 0].astype(float).values   # 1, 2, 3, ..., 360
sinex = data.iloc[:, 1].astype(float).values    # sin(1°), sin(2°), ...

# ===== 关键：输入归一化 =====
# [1, 360] → [-1, 1]，TANH 在线性区工作
x_min, x_max = raw_x.min(), raw_x.max()         # 1.0, 360.0
x_norm = (raw_x - (x_min + x_max) / 2.0) / ((x_max - x_min) / 2.0)

print(f"归一化公式: x_norm = (x - 180.5) / 179.5")
print(f"STM32端: *input = (angle_deg - 180.5f) / 179.5f\n")

# 建立模型
model = tf.keras.Sequential()
model.add(tf.keras.layers.Dense(units=20, activation='tanh', input_shape=(1,)))
model.add(tf.keras.layers.Dense(units=10, activation='tanh'))
model.add(tf.keras.layers.Dense(units=1))
model.summary()

# 学习率自动衰减
lr_cb = tf.keras.callbacks.ReduceLROnPlateau(
    monitor='loss', factor=0.5, patience=500, min_lr=1e-6, verbose=1)
es_cb = tf.keras.callbacks.EarlyStopping(
    monitor='loss', patience=1000, min_delta=1e-7, verbose=1)

model.compile(
    optimizer=tf.keras.optimizers.AdamW(0.001),
    loss=tf.keras.losses.mse,
    metrics=[tf.keras.metrics.mse])

history = model.fit(
    x=x_norm, y=sinex,
    epochs=5000,
    callbacks=[lr_cb, es_cb],
    verbose=2)

# 全范围评估
print(f"\n{'角度':>6}  {'true':>8}  {'预测':>8}  {'误差':>8}")
max_err = 0
worst_deg = 0
for deg in range(0, 361, 10):
    xn = (deg - 180.5) / 179.5
    true_v = np.sin(np.radians(deg))
    pred = model.predict(np.array([[xn]]), verbose=0)[0][0]
    err = abs(pred - true_v)
    if err > max_err:
        max_err = err
        worst_deg = deg
    m = " <--" if err > 0.05 else ""
    print(f"{deg:6}  {true_v:8.4f}  {pred:8.4f}  {err:8.4f}{m}")

print(f"\n最大误差: {max_err:.4f} @ {worst_deg}°")
print(f"最终 loss: {history.history['loss'][-1]:.8f}")

# 保存 h5
model.save('./sine_calcu.h5')

# 转换 tflite
load_model = tf.keras.models.load_model('./sine_calcu.h5')
converter = tf.lite.TFLiteConverter.from_keras_model(load_model)
tflite_model = converter.convert()
with open('./sine_calcu.tflite', 'wb') as f:
    f.write(tflite_model)

print("\n已保存: ./sine_calcu.h5, ./sine_calcu.tflite")
