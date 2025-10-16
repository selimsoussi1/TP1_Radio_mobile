import matplotlib.pyplot as plt

# Lecture du fichier
temps = []
valeurs = []

with open("output.txt", "r") as f:
    for line in f:
        line = line.strip()
        if not line or "RxDrop" in line:
            continue
        parts = line.split()
        if len(parts) == 2:
            try:
                t, v = float(parts[0]), float(parts[1])
                temps.append(t)
                valeurs.append(v)
            except ValueError:
                pass

# Tracer la courbe
plt.figure(figsize=(10,5))
plt.plot(temps, valeurs, marker='o', linestyle='-', color='b')
plt.xlabel("Temps (s)")
plt.ylabel("Valeur")
plt.title("Courbe générée à partir de fifth.cc")
plt.grid(True)
plt.show()

