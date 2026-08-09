# Shade Automation V4 — Application Requirements

## 1. Purpose

This document defines the functional requirements for Shade Automation V4.

The application controls six powered window shades in the coach and provides local and external control while preventing electronic commands from creating an unsafe shade position when the coach may be in motion.

This is a requirements document. It intentionally does not define the C++ architecture, classes, namespaces, data structures, or implementation.

---

## 2. Physical System

- The coach has **6 powered shades**.
- The shades are arranged as **3 pairs**.
- Each pair consists of:
  - 1 day shade
  - 1 night shade
- Each pair of shades is controlled by **one ESP32 controller**.
- The two shades on a controller operate independently.
- All shade actions are asynchronous; operation of one shade must not prevent another shade from being operated.

- All three shade controllers use the same shade-control hardware pinout. The only established hardware difference between the controllers is the environmental temperature sensor."

### Driver's seat: DS18B20
### Windshield: DS18B20
### Passenger seat: DHT22
 - Shade switch, motor-control, motor-driver, and parking-brake GPIO assignments are common to all three controllers.

---

## 3. Basic Shade Movement

Each individual shade must:

- Raise.
- Lower.
- Stop at any point between fully open and fully closed.
- Be capable of being commanded from a physical switch or an electronic command.

### Position convention

- **0% down** = fully open.
- **100% down** = fully closed.

---

## 4. Physical Switch Behavior

Physical/manual switches are momentary switches with no latching or locking mechanism.

### Single-click behavior

A single switch action must be capable of:

- Starting a shade toward its appropriate fully-open or fully-closed position.
- Stopping a shade that is currently moving.

### Held switch behavior

If a physical switch is held down:

- Power continues to be applied to the shade for as long as the switch remains held.
- The application does not need to protect the motor from indefinite switch closure at either endpoint because the shade motor contains internal limit switching that removes power at the fully-open and fully-closed limits.

The internal motor limit switching operates in both directions.

### Physical switch at an endpoint

- A physical switch may remain held after a shade reaches fully open or fully closed.
- The motor's internal limit switch removes motor power at the endpoint.
- No software protection is required to prevent motor damage from a held physical switch at an endpoint.

---

## 5. Electronic/Web/Node-RED Command Behavior

Electronic commands can originate from the web interface or Node-RED.

When a shade command is received, the first question is whether the shade is currently moving.

### Shade is stopped

If the shade is not moving:

- A command to raise starts raising.
- A command to lower starts lowering.

### Shade is moving

If the shade is moving and a command is received:

- If the command requests the **same direction** as the current movement, the shade is stopped.
- If the command requests the **opposite direction**:
  1. Stop the shade.
  2. Wait the required number of milliseconds.
  3. Start the shade in the opposite direction.

Thus an electronic command while moving either cancels the current movement or reverses it.

### Electronic command at an endpoint

- A shade already fully open must not be driven farther upward.
- A shade already fully closed must not be driven farther downward.
- The external control state must reflect that the corresponding movement button is unavailable.

---

## 6. Shade Position Estimation

The shades do not have a position sensor.

Position is estimated from motor run time.

For each shade:

- The time required to travel fully open is determined manually.
- The time required to travel fully closed is determined manually.
- Open and close travel times are not assumed to be identical.
- The application estimates position from the amount of time the motor has been running and the previously known position.
- Short movements accumulate into an approximate position.

The resulting position is approximate rather than an exact physical measurement.

### Position persistence

Whenever a stop command is processed:

- The estimated shade position is saved to disk.
- The saved position is available after reboot.

If no saved position exists for a shade at boot:

- The shade defaults to **0% down (fully open)**.

### Position while moving

- The application continuously maintains the estimated shade position while a shade is moving.
- Position updates do not wait for the shade to stop.

### Power loss while moving

If the controller loses power or crashes while a shade is moving:

- The stored position may be incorrect because the last known position was not necessarily the actual position when power was lost.

The position can be recovered manually by:

- Holding the physical switch so the shade completes a cycle, or
- Driving the shade in the opposite direction using an electronic switch.

---

## 7. Controller Environmental Monitoring

The shade controllers provide environmental temperature monitoring.

- Two of the three controllers use an onboard **DS18B20** temperature sensor.
- The third controller, which was the first controller developed, uses a **DHT22 temperature/humidity sensor**.
- Current temperature readings are sent to the external system when the temperature changes.
- A temperature log reading is sent every **5 minutes**.
- The DHT22 controller also has humidity available from its sensor.

The exact external interface and required handling of these readings will be defined separately from the shade-control behavior.

## 8. Controller Reboot Notification

- A notification of a controller reboot is sent to Node-RED.
- The exact reboot-notification payload and external interface will be defined separately.

## 9. Configuration and Operational Settings

The application must provide a way to update operational settings, including:

- Day-shade parking-brake-on maximum run time (`pkBkOnTimeToRun`).
- Night-shade parking-brake-on maximum run time (`pkBkOnTimeToRun`).
- Sensor-check interval (`senChkIntv`).
- Sensor-log interval (`senLogIntv`).

The exact configuration interface and persistence mechanism will be defined separately.

## 10. Command and Controller Boundaries

- All shade-movement commands must be rejected while the controller is in its booting process.
- A controller must not process shade commands intended for another shade controller.
- Each controller must ensure that only commands addressed to the shades it controls are acted upon.

## x. External Status and User Interface State

The application must communicate meaningful shade-action/status statements to the web interface and Node-RED, including states such as:

- Shade is going down.
- Shade is going up.
- Shade is stopped.
- Other application-defined shade status/action states as required.

The application must also provide the current live/dead (enabled/disabled) state of shade control buttons to the web interface and Node-RED.

Button availability must reflect the current shade state. For example:

- When a shade is fully open, its **Up** button is disabled/gray.
- When a shade is fully closed, its **Down** button is disabled/gray.
- Other button states must similarly reflect whether the requested action is currently valid.

The exact status vocabulary and external message format will be defined separately.

### Live button state during movement

- Button states are updated while a shade is moving.
- Web and Node-RED do not wait for a shade to stop before receiving updated button-state information.
- Button availability reflects the shade's current estimated position and movement state.

---

## 12. Parking Brake / In-Motion Safety

The parking brake determines whether the coach should be treated as potentially in motion.

### Parking brake applied

When the parking brake is applied:

- The allowable shade-down limit returns to **100%**.
- No shade movement is commanded solely because the parking brake was applied.

### Parking brake released

When the parking brake is released:

- The coach is considered **in motion** for shade-safety purposes.
- Each shade has an **in-motion maximum-down position**.
- Electronic switches/commands must not lower a shade beyond its in-motion maximum-down position.
- If a shade is already farther down than its in-motion maximum-down position when the parking brake is released, the shade must be raised to the in-motion maximum-down position.

The purpose of this behavior is to prevent an electronic command from lowering a shade far enough to obstruct the driver's vision while the bus may be moving.

### Parking-brake changes while a shade is moving

- A change in parking-brake state must be applied to a shade that is already moving.
- When the parking brake is released, a shade moving downward must not continue beyond the newly applicable in-motion maximum-down position.
- When the parking brake is applied, the allowable down position returns to the full-travel limit; applying the brake does not itself command a shade to move.

### Physical switches while in motion

The physical switches are located in the cockpit.

- The driver controls the windshield and driver-window shades.
- The passenger controls the passenger shade.
- Physical switches are not restricted by the in-motion maximum-down limit.
- A physical switch may lower a shade beyond the in-motion maximum-down position while the parking brake is released.
- This is intentional because there are legitimate circumstances while traveling when a shade may need to be lowered farther, such as blocking sunlight from the driver's eyes.
- The design does not attempt to prevent malicious use of the physical switches.
- The design assumes that people operating the physical controls are trusted to exercise appropriate judgment.

The in-motion maximum-down position is therefore an electronic-control safety constraint, not a physical-switch constraint.

---

## 13. External Shade Status

The application must provide web and Node-RED with the approximate percentage that each shade is down.

The position convention is:

- `0%` = fully open
- `100%` = fully closed

---

## 14. Group Controls

External controls exist for operating groups of shades.

At minimum:

- One switch can operate the **3 day shades** as a group.
- One switch can operate the **3 night shades** as a group.

When a group command is issued:

- All three shades in the selected group receive the requested action.
- Each shade independently applies the normal shade-command rules.
- If a shade is stopped, it starts in the requested direction.
- If a shade is already moving in the requested direction, it stops.
- If a shade is moving in the opposite direction, it stops, waits the required reversal delay, and then starts in the requested direction.
- The three shades do not need to be at the same position or in the same movement state for the group command to be issued.

---

## 15. Independence and Concurrency

Each controller manages its two shades independently.

Requirements:

- Both shades may operate at the same time.
- A command for one shade must not block operation of the other shade.
- Shade actions are asynchronous.
- Any shade may be acted upon while another shade is moving.

---

## 16. Requirements Still To Be Defined

The following areas remain to be clarified or recovered from the production application:

- Exact behavior and reporting associated with position calibration/recovery.
- Exact configuration and naming of each shade's in-motion maximum-down setting.
- Complete list of external status/action statements that must be exposed to Node-RED and the web interface.
- Complete list of externally configurable application settings.
- Any additional production behaviors discovered during the V3 review that have not yet been confirmed as V4 requirements.

No separate application-level error/fault requirement has been identified at this point.

---

## 17. Persistent Local Storage

The application requires persistent local storage for information that must survive a controller reboot, including:

- Shade position.
- Application configuration and operational settings.

Current production controllers use microSD cards. This originated from an earlier design in which local disks were used for logging and log retention.

Logs are now hosted on the Node-RED server, substantially reducing local storage requirements.

V4 should use the project's standard **LittleFS** storage mechanism rather than requiring microSD for normal operation.

Local storage is not intended to be the primary long-term home for application logs.

---

## 18. Design Boundary

This document describes what the Shade Automation application must do.

It does not yet prescribe:

- C++ classes
- namespaces
- structures
- pointers
- files
- function names
- EI interfaces
- Node-RED implementation
- Web implementation

Those decisions will be derived after the required behavior is sufficiently defined.
