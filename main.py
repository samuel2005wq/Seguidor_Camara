from maix import camera, display, image, nn, app, uart
import os

model_path = "/root/models/model-264135.maixcam/model_264135.mud"
classifier = nn.Classifier(model=model_path)
cam = camera.Camera(classifier.input_width(), classifier.input_height(), classifier.input_format())
disp = display.Display()

devices = uart.list_devices()
print("Puertos disponibles:", devices)
serial = uart.UART(devices[0], 9600)

while not app.need_exit():
    img = cam.read()
    res = classifier.classify(img)
    max_idx, max_prob = res[0]
    label = classifier.labels[max_idx]
    msg = f"{max_prob:5.2f}: {label}"
    img.draw_string(10, 10, msg, image.COLOR_RED)
    disp.show(img)

    if max_prob > 0.85:
        serial.write_str(label + "\n")



