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

---

## Signalverarbeitung (DSP)

Das Plugin unterstützt 4 Grundwellenformen:

- **Sine (Wolf/Heulen)**:
  - Vibrato (Frequenzmodulation per LFO)
  - Tremolo (Amplitude-Modulation per LFO)
  - Chorus (mehrstimmiger Heuleffekt)
  - Dynamisches Filter (Cutoff moduliert durch Hüllkurve)

- **Saw (Bär/Grollen)**:
  - Comb Filter (mit Delay und Feedback)
  - Formant Filter (vokalartige Resonanzen)
  - Waveshaper Distortion (Drive & Shape, deaktivierbar)

- **Square (Hund/Bellen)**:
  - Punch-Hüllkurve (Attack-Boost)
  - Bitcrusher (Sample- und Bitratenreduktion)
  - Bark-Filter (Bandpass mit Hüllkurvenmodulation)

- **Triangle (Vogel/Zwitschern)**:
  - Pitch Glide (Portamento)
  - Chirp (AM-Modulation)
  - Echo (delay-basiert mit Zeit und Mix)

Zusätzlich:
- Eine ADSR-Hüllkurve wird für jede Stimme angewendet.
- Die Parameter sind über `AudioProcessorValueTreeState` angebunden.
- Alle Effekte sind über das GUI steuerbar und automatisierbar.


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
    → Initialisierung von Frequenz, Phase, Effekthüllkurven
    → Sample-Loop:
        → Grundwellenform-Generierung (Sinus, Sägezahn, Rechteck, Dreieck)
        → Modulationseffekte (z. B. Glide, Vibrato, Tremolo, Chirp)
        → Filteranwendungen (z. B. Bark-Filter, Formantfilter, dynamischer Bandpass)
        → Waveshaping / Distortion (falls aktiviert)
        → Bitcrusher / Echo / Comb-Filter (wellenformspezifisch)
        → Anwendung der ADSR-Hüllkurve auf Amplitude
    → Schreiben der Samples in den Audiopuffer

