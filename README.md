# Ignore Touchpad

**Ignore Touchpad** is a utility suite for [Haiku OS](http://www.haiku-os.org) that allows users to selectively ignore input from specific pointing devices, such as touchpads or external mice.  

Its main purpose is to prevent accidental cursor movements and clicks while typing, for example when your thumbs touch the laptop's touchpad.

---

## 📖 Documentation

The full Doxygen-generated documentation for **Ignore Touchpad** is available online:

➡ [**View Documentation**](https://ahitech.github.io/IgnoreTouchpad/)

This documentation includes:
- Detailed class and method descriptions
- Data structures used by the Addon, GUI, CLI, and Settings modules
- Cross-references and diagrams for project architecture

---

## ✨ Features

- Temporarily **ignore input from selected devices** (touchpads or mice).
- Never blocks keyboards or other non-pointing input devices.
  - But this may be changed in the future, especially if users request this functionality.
- System-wide effect, implemented via `BInputDevice::Stop()`.
- Global keyboard shortcut to **unignore all devices** instantly. Assuming keyboard is never affected by this program, a shortcut should be a safe way to revert current status and make ~~Haiku great~~ all devices available again.
- Optional **Deskbar replicant** to show and manage current ignore status.
- CLI and GUI interface.

---

## 📂 Project Structure

📂 `IgnoreTouchpad`

├── 📂 `CLI` - command-line utility for listing the devices, setting and resetting device status

├── 📂 `GUI` - GUI utility for listing the devices, setting and resetting device status. It also hosts the Deskbar replicant.

├── 📂 `Settings` - a small shared library for reading and modifying the settings file.

├── 📂 `Addon` - the input server filter that does all of the work of ignoring messages from ignored devices.

├── 📄 `License.md` - for legal purposes

├── 📄 `README.md` - duh

├── 🛠️ `make` - script for compiling the whole damn thing

└── TBD


---

## ⚙️ How It Works

1. The CLI utility builds the list of all pointing devices registered in the system.
2. If a device is set to "disabled", all input events from that device are silently discarded.
3. The GUI (Deskbar replicant `Ignore Touchpad`) allows you to:
   - View all connected pointing devices.
   - Toggle "Ignore input" per device.
   - Restore all devices to the default (unignored) state with one button.
4. The CLI (`ignore_touchpad`) supports interactive mode, scripting and automation:

```bash
ignore_touchpad help
ignore_touchpad list
ignore_touchpad disable <device_id>
ignore_touchpad enable <device_id>
ignore_touchpad enable_all
ignore_touchpad interactive
```

---

## 🔐 Safety Considerations

- The **last active pointing device** cannot be ignored (checkbox is disabled in UI, the CLI command will fail).  
- Rebooting re-enables all devices.

---

## 🛠️ Building

This project uses **autotools**:

```bash
git clone git@github.com:yourusername/ignore_mouse.git
cd ignore_mouse
./configure
make
make install
```

---

## 📄 License

This project is licensed under the MIT License. See License.md.

---

## ✅ Implemented / 🚧 ToDo

✅ Project structure and initial design
✅ MIT License and README
✅ Automatically generated and published documentation in Doxygen format
✅ CLI utility ignore_touchpad
🚧 Deskbar replicant with status icon
🚧 Translations (CatKeys)
🚧 Install/uninstall scripts
🚧 Scenarios
  - Always disable touchpad when a known pointing device such as external mouse is connected
  - Automatically enable touchpad when the external pointing device is disconnected

Happy typing without accidental touchpad clicks! 
