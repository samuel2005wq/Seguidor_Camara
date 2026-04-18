
from maix import camera, display, image, nn, app, comm
import struct, os

APP_CMD_CLASSIFY_RES = 0x02

def encode_objs(res):
    '''
        encode objs info to bytes body for protocol
        2B max idx, 4B prob, 2B second idx, 4B prob, 2B third idx, 4B prob
    '''
    body = b''
    for obj in res:
        body += struct.pack("<Hf", obj[0], obj[1])
    return body

model_path = "model_264492.mud"
if not os.path.exists(model_path):
    model_path = "/root/models/maixhub/264492/model_264492.mud"
classifier = nn.Classifier(model=model_path)

cam = camera.Camera(classifier.input_width(), classifier.input_height(), classifier.input_format())
disp = display.Display()

p = comm.CommProtocol(buff_size = 1024)

while not app.need_exit():
    # msg = p.get_msg()

    img = cam.read()
    res = classifier.classify(img)

    body = encode_objs(res)
    p.report(APP_CMD_CLASSIFY_RES, body)

    max_idx, max_prob = res[0]
    msg = f"{max_prob:5.2f}: {classifier.labels[max_idx]}"
    img.draw_string(10, 10, msg, image.COLOR_RED)

    img = img.resize(disp.width(), disp.height(), image.Fit.FIT_CONTAIN)
    disp.show(img)

