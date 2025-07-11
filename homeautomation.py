import cv2
import mediapipe as mp
import firebase_admin
from firebase_admin import credentials, db

# -------------------- Firebase Setup --------------------
cred = credentials.Certificate("serviceAccountKey.json")
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://homeautomation-94e12-default-rtdb.asia-southeast1.firebasedatabase.app/'  # Replace with your DB URL
})

light_ref = db.reference("light")
fan_ref = db.reference("fan")

# -------------------- MediaPipe Setup --------------------
mp_hands = mp.solutions.hands
mp_drawing = mp.solutions.drawing_utils
hands = mp_hands.Hands(max_num_hands=1, min_detection_confidence=0.7)

# Finger tip landmarks
finger_tips = [4, 8, 12, 16, 20]

# Start webcam
cap = cv2.VideoCapture(0)
prev_command = ""

while True:
    ret, frame = cap.read()
    if not ret:
        break

    frame = cv2.flip(frame, 1)
    rgb_frame = cv2.cvtColor(frame, cv2.COLOR_BGR2RGB)
    results = hands.process(rgb_frame)

    command = "NONE"

    if results.multi_hand_landmarks:
        hand_landmarks = results.multi_hand_landmarks[0]
        lm = hand_landmarks.landmark

        # Detect finger states (True = up)
        fingers = []

        # Thumb (x comparison for lateral movement)
        fingers.append(lm[finger_tips[0]].x > lm[finger_tips[0] - 1].x)

        # Index to Pinky
        for tip in finger_tips[1:]:
            fingers.append(lm[tip].y < lm[tip - 2].y)

        # Map to gestures
        if fingers == [True, True, True, True, True]:
            command = "LIGHT_ON"
            light_ref.set("ON")

        elif fingers == [False, False, False, False, False]:
            command = "LIGHT_OFF"
            light_ref.set("OFF")

        elif fingers == [False, True, False, False, True]:  # Index + Pinky
            command = "FAN_ON"
            fan_ref.set("ON")

        elif fingers == [False, True, True, False, False]:  # Index + Middle
            command = "FAN_OFF"
            fan_ref.set("OFF")

        mp_drawing.draw_landmarks(frame, hand_landmarks, mp_hands.HAND_CONNECTIONS)

    # Send only when changed
    if command != prev_command and command != "NONE":
        print(f"Gesture Detected: {command}")
        prev_command = command

    # UI Display
    cv2.putText(frame, f'Command: {command}', (10, 60),
                cv2.FONT_HERSHEY_SIMPLEX, 1.2, (255, 0, 0), 3)

    cv2.imshow("Gesture-Controlled Home Automation", frame)

    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()