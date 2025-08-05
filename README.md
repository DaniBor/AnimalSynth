# Animal Synth

Der tierbasierte Synthesizer! Erstelle tierähnliche Laute mithilfe verschiedener Wellenformen mit jeweils eigenen Effekten.

---

## Inhaltsverzeichnis

1. [Architekturübersicht](#architekturübersicht)
2. [Signalverarbeitung (DSP)](#signalverarbeitung-dsp)
3. [GUI-Aufbau](#gui-aufbau)
4. [Signalfluss](#signalfluss)
5. [Besonderheiten](#besonderheiten)
6. [Verwendete externe Libraries](#verwendete-externe-libraries)

---

## Architekturübersicht

### Module

- `PluginProcessor.cpp/.h` – Zentrale Verarbeitung und Parameterverwaltung
- `PluginEditor.cpp/.h` – GUI-Darstellung und Benutzerinteraktion
- `AnimalVoice.cpp/.h` – SynthesizerVoice für polyphonen Betrieb (vorbereitet)
- `ScaledVisualiserComponent` – Echtzeit-Wellenformanzeige
- `AnimationDisplayComponent` – Darstellung animierter Bilder basierend auf dem Hüllkurvenlevel
- `FX Panels` – Separate Panels für Sine, Saw, Square und Triangle Wellenformen

### Klassenstruktur

- `AnimalSynthAudioProcessor` – Hauptprozessor
- `AnimalSynthAudioProcessorEditor` – Editor mit Panelverwaltung
- `ScaledVisualiserComponent` – Oszilloskopartige Anzeige
- `AnimationDisplayComponent` – Bilddarstellung anhand der ADSR
- `AnimalVoice` – Vorbereitete Klasse für zukünftige polyphone Nutzung

---

## Signalverarbeitung (DSP)

Das Plugin unterstützt 4 Grundwellenformen:

- **Sine (Wolf/Heulen)**:
  - Vibrato
  - Flutter (leichtes periodisches Verstimmen)
  - Tremolo
  - Dynamisches Filter

- **Saw (Bär/Grollen)**:
  - Distortion mit Tone-Kontrolle
  - Dynamischer Bandpass-Sweep

- **Square (Hund/Bellen)**:
  - Punch-Hüllkurve (Attack-Boost)
  - Bitcrusher
  - Bark-Filter (resonanter Bandpass mit Hüllkurve)

- **Triangle (Vogel/Zwitschern)**:
  - Pitch Glide
  - Chirp (AM-Modulation)
  - Echo (delay-basiert)

Zusätzlich:
- ADSR-Hüllkurve wird für jede Stimme angewendet
- Parameter über `AudioProcessorValueTreeState` angebunden

---

## GUI-Aufbau

- Zentrale Steuerung: Auswahl der Wellenform, ADSR-Hüllkurve
- Effekte:
  - Jeder Wellentyp hat eine eigene Panel-Seite mit max. 6 Slidern
- Visualisierung:
  - Echtzeit-Oszilloskop (ScaledVisualiserComponent)
  - Tieranimationen (AnimationDisplayComponent) basierend auf Hüllkurvenlevel
- Hintergrundbilder und optisches Styling der Panels

---

## Signalfluss

```text
MIDI Note On →
    → Bestimmung der Wellenform
    → Initialisierung Frequenz, Phase, Filter
    → Sample-Loop:
        → Wellenform-Generierung
        → Effektberechnung (z. B. Glide, Vibrato, etc.)
        → Filter/Modulation
        → Anwendung der Hüllkurve
        → (Optional) Bitcrusher, Echo etc.
    → Ausgabe an Audiopuffer
