# P01-A verification / design review

## Netlist review
- [ ] GP4/GP5 are the only I2C controller nets.
- [ ] LIS2DU12/LIS3DH, AS7343 and DRV2605L share the intended bus.
- [ ] No I2C address collision.
- [ ] GP16/17/18/19 are SD MISO/CS/SCK/MOSI respectively.
- [ ] USER_TEST is active-low and cannot float.
- [ ] FAULT_IN is active-low and fails toward fault when disconnected.
- [ ] INTERLOCK_PROOF, TIMER_A_OK and TIMER_B_OK default to not-permitted.
- [ ] LOAD_EN defaults LOW.
- [ ] LOAD_FB is independently sensed.
- [ ] Test load is not powered from a GPIO.

## Firmware review
- [ ] Load control is forced LOW before peripheral initialization.
- [ ] Self-test fails closed.
- [ ] ACTIVE requires all proofs.
- [ ] ACTIVE is bounded by a fixed timeout.
- [ ] Any proof loss immediately disables LOAD_EN.
- [ ] Feedback disagreement causes FAULT.
- [ ] COOLDOWN prevents immediate retrigger.
- [ ] FAULT holds outputs safe until deliberate reset/recovery.

## Cirkit review
- [ ] One coherent schematic.
- [ ] No required dangling pins.
- [ ] All grounds connected.
- [ ] 3V3 decoupled.
- [ ] Test points present.
- [ ] Timer A and timer B represented separately.
- [ ] Interlock represented separately from firmware permission.
- [ ] Fault path represented separately.
- [ ] Simulation load is low-voltage only.

## Physical gate
Passing Cirkit does not certify the real optical/high-power section. Before physical connection, separately verify regulator capacity, switch SOA/current rating, timer component values, connector ratings, thermal design, PCB creepage/clearance, emitter electrical limits and enclosure requirements.
