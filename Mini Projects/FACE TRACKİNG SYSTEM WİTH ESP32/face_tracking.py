import cv2
import mediapipe as mp
import requests
import time
import numpy as np
import threading
import tkinter as tk
from tkinter import messagebox
from queue import Queue

# ======================
# ESP32-CAM 
# ======================
ESP_IP = "IP..."
STREAM_URL = f"http://{ESP_IP}:81/"
SERVO_URL = f"http://{ESP_IP}/servo"
LED_URL = f"http://{ESP_IP}/led"

# ======================
# PID / Servo
# ======================
Kp_pan, Kp_tilt = 0.18, 0.12
Kd_tilt = 0.08
pan, tilt = 90, 90
PAN_MIN, PAN_MAX = 20, 160
TILT_MIN, TILT_MAX = 50, 130
alpha, MAX_DELTA = 0.6, 10
SEND_INTERVAL = 0.005
last_send = 0
last_err_x, last_err_y = 0, 0
led_on = False
center_offset_y = 15
track_mode = "face"
stop_flag = False

session = requests.Session()
def clamp(v, a, b): return max(a, min(b, v))

# ======================
# Mediapipe Setup
# ======================
mp_drawing = mp.solutions.drawing_utils

holistic = mp.solutions.holistic.Holistic(
    static_image_mode=False,
    model_complexity=1,
    smooth_landmarks=True,
    min_detection_confidence=0.5,
    min_tracking_confidence=0.5
)

hands = mp.solutions.hands.Hands(
    static_image_mode=False,
    max_num_hands=1,
    min_detection_confidence=0.4,
    min_tracking_confidence=0.5,
    model_complexity=0
)

# ======================
# Frame Queue
# ======================
frame_queue = Queue(maxsize=2)

def camera_thread():
    global stop_flag
    cap = cv2.VideoCapture(STREAM_URL)
    if not cap.isOpened():
        messagebox.showerror("error", "ESP32-CAM couldnt run!")
        stop_flag = True
        return

    while not stop_flag:
        ret, frame = cap.read()
        if not ret:
            time.sleep(0.01)
            continue
        frame = cv2.flip(frame, 1)
        frame = cv2.resize(frame, (320, 240))
        if not frame_queue.full():
            frame_queue.put(frame)
        else:
            try:
                frame_queue.get_nowait()
            except:
                pass
            frame_queue.put(frame)
    cap.release()

# ======================
# Tracking Thread
# ======================
def tracking_thread():
    global pan, tilt, last_err_x, last_err_y, last_send, stop_flag, track_mode
    alpha_hand = 0.7

    while not stop_flag:
        if frame_queue.empty():
            time.sleep(0.005)
            continue

        frame = frame_queue.get()
        h, w, _ = frame.shape
        rgb = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)

        # ===== Face Takibi =====
        if track_mode == "face":
            results = holistic.process(rgb)
            if results.face_landmarks:
                face_landmarks = results.face_landmarks
                head_ids = [10,152,234,454,127,356,46,276,172,406,1,9]
                hx = np.mean([face_landmarks.landmark[i].x for i in head_ids]) * w
                hy = np.mean([face_landmarks.landmark[i].y for i in head_ids]) * h
                err_x = hx - w/2
                err_y = (hy - h/2) - center_offset_y
                err_diff_y = err_y - last_err_y
                last_err_x, last_err_y = err_x, err_y

                cv2.circle(frame, (int(hx), int(hy)), 5, (0,255,0), -1)
                mp_drawing.draw_landmarks(frame, face_landmarks,
                                          mp.solutions.holistic.FACEMESH_CONTOURS,
                                          landmark_drawing_spec=None,
                                          connection_drawing_spec=mp_drawing.DrawingSpec(color=(0,255,0), thickness=1))
            else:
                err_x, err_y = last_err_x*0.5, last_err_y*0.5
                err_diff_y = 0

        # ===== Hand Takibi =====
        elif track_mode == "hand":
            results = hands.process(rgb)
            if results.multi_hand_landmarks:
                hand_landmarks = results.multi_hand_landmarks[0]
                visible_lms = [lm for lm in hand_landmarks.landmark]
                hx = np.mean([lm.x for lm in visible_lms]) * w
                hy = np.mean([lm.y for lm in visible_lms]) * h

                last_err_x = alpha_hand*(hx - w/2) + (1-alpha_hand)*last_err_x
                last_err_y = alpha_hand*(hy - h/2) + (1-alpha_hand)*last_err_y
                err_x, err_y = last_err_x, last_err_y
                err_diff_y = 0

                mp_drawing.draw_landmarks(frame, hand_landmarks, mp.solutions.hands.HAND_CONNECTIONS)
                cv2.circle(frame, (int(hx), int(hy)), 8, (0,0,255), -1)
            else:
                last_err_x *= 0.8
                last_err_y *= 0.8
                err_x, err_y = last_err_x, last_err_y
                err_diff_y = 0

        # ===== Servo CALCULATIONS=====
        target_pan = clamp(pan + Kp_pan*err_x, PAN_MIN, PAN_MAX)
        target_tilt = clamp(tilt + Kp_tilt*err_y + Kd_tilt*err_diff_y, TILT_MIN, TILT_MAX)
        pan = int(pan + clamp(target_pan - pan, -MAX_DELTA, MAX_DELTA) * alpha)
        tilt = int(tilt + clamp(target_tilt - tilt, -MAX_DELTA, MAX_DELTA) * alpha)

        if time.time() - last_send > SEND_INTERVAL:
            try:
                session.get(SERVO_URL, params={"pan": pan, "tilt": tilt}, timeout=0.25)
                last_send = time.time()
            except:
                pass

        # ===== WIEW =====
        color = (0,255,0) if track_mode=="face" else (0,0,255)
        label = "follow: face" if track_mode=="face" else "follow: hand"
        cv2.putText(frame, label, (10,30), cv2.FONT_HERSHEY_SIMPLEX, 0.7, color, 2)
        cv2.putText(frame, f"pan={pan} tilt={tilt}", (10,60), cv2.FONT_HERSHEY_SIMPLEX, 0.7, (255,255,0), 2)

        cv2.imshow("ESP32-CAM Tracking", frame)
        if cv2.waitKey(1) & 0xFF == ord('q'):
            stop_flag = True

# ======================
# LED Button
# ======================
def toggle_led():
    global led_on
    new_state = not led_on
    try:
        requests.get(LED_URL, params={"state": int(new_state)}, timeout=1)
        led_on = new_state
        btn_led.config(text=("LED off" if led_on else "LED on"))
    except:
        messagebox.showerror("error", "it couldnt connect to esp32!")

# ======================
# Mod Button
# ======================
def toggle_mode():
    global track_mode
    if track_mode=="face":
        track_mode="hand"
        btn_mode.config(text="Mod: hand tracking (change)")
    else:
        track_mode="face"
        btn_mode.config(text="Mod: face tracking (change))")

# ======================
# Tkinter GUI
# ======================
root = tk.Tk()
root.title("ESP32-CAM Kontrol")
root.geometry("280x220")

btn_led = tk.Button(root, text="LED Aç", font=("Arial",12), command=toggle_led)
btn_led.pack(pady=10)

btn_mode = tk.Button(root, text="Mod: face tracking (change)", font=("Arial",12), command=toggle_mode)
btn_mode.pack(pady=10)

tk.Label(root, text="to close press Q ").pack(pady=10)

# ======================
# Thread start
# ======================
threading.Thread(target=camera_thread, daemon=True).start()
threading.Thread(target=tracking_thread, daemon=True).start()

root.mainloop()
stop_flag = True
