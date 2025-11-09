import matplotlib.pyplot as plt
import numpy as np
import csv
from scipy import stats
import matplotlib.ticker as ticker

# Configuration du style professionnel
plt.style.use('seaborn-v0_8-whitegrid')
plt.rcParams['font.family'] = 'DejaVu Sans'
plt.rcParams['font.size'] = 12
plt.rcParams['axes.labelsize'] = 14
plt.rcParams['axes.titlesize'] = 16
plt.rcParams['xtick.labelsize'] = 12
plt.rcParams['ytick.labelsize'] = 12
plt.rcParams['legend.fontsize'] = 12
plt.rcParams['figure.titlesize'] = 18

def load_and_analyze_data():
    """Charge et analyse les données de fenêtre TCP et les pertes"""
    time_ms = []
    cwnd = []
    loss_times = []
    
    # Charger les données de fenêtre TCP
    try:
        with open('tcp_cwnd_data.csv', 'r') as f:
            reader = csv.reader(f)
            next(reader)  # Skip header
            for row in reader:
                if row and len(row) >= 2:  # Éviter les lignes vides
                    time_ms.append(float(row[0]))
                    cwnd.append(int(row[1]))
        print(f"Données TCP chargées: {len(time_ms)} points")
    except FileNotFoundError:
        print("❌ Fichier tcp_cwnd_data.csv non trouvé")
        return None, None, None
    except Exception as e:
        print(f"❌ Erreur lecture tcp_cwnd_data.csv: {e}")
        return None, None, None
    
    # Si pas de données CWND, générer des données simulées
    if len(time_ms) == 0:
        print("⚠️  Aucune donnée TCP trouvée, génération de données simulées...")
        time_ms, cwnd = generate_simulated_data()
    
    # Charger les données de pertes
    try:
        with open('tcp_loss_data.csv', 'r') as f:
            reader = csv.reader(f)
            next(reader)  # Skip header
            for row in reader:
                if row and len(row) >= 2:  # Éviter les lignes vides
                    loss_times.append(float(row[0]))
        print(f"✓ Pertes détectées: {len(loss_times)} paquets")
    except FileNotFoundError:
        print("⚠️  Fichier tcp_loss_data.csv non trouvé")
    except Exception as e:
        print(f"⚠️  Erreur lecture tcp_loss_data.csv: {e}")
    
    return np.array(time_ms), np.array(cwnd), np.array(loss_times)

def generate_simulated_data():
    """Génère des données TCP réalistes pour la démonstration"""
    # Simulation sur 20 secondes
    time_ms = np.linspace(1000, 20000, 500)  # 1s à 20s
    
    # Comportement TCP réaliste
    cwnd = []
    current_cwnd = 1.0
    ssthresh = 16.0
    
    for t in time_ms:
        # Slow Start vs Congestion Avoidance
        if current_cwnd < ssthresh:
            current_cwnd *= 1.4  # Croissance exponentielle
        else:
            current_cwnd += 0.8  # Croissance linéaire
        
        # Pertes aléatoires réalistes
        if np.random.random() < 0.02 and current_cwnd > 8:
            current_cwnd = max(1, current_cwnd * 0.5)  # Réduction TCP
            ssthresh = current_cwnd
        
        # Limite réaliste
        current_cwnd = min(current_cwnd, 64)
        cwnd.append(current_cwnd)
    
    print(f"✓ Données simulées générées: {len(time_ms)} points")
    return time_ms, cwnd

def analyze_tcp_behavior(time_ms, cwnd, loss_times):
    """Analyse approfondie du comportement TCP"""
    if len(cwnd) == 0:
        return {}, []
    
    # Conversion en secondes pour l'analyse
    time_s = time_ms / 1000.0
    
    # Statistiques de base
    stats = {
        'max_cwnd': np.max(cwnd),
        'min_cwnd': np.min(cwnd),
        'mean_cwnd': np.mean(cwnd),
        'std_cwnd': np.std(cwnd),
        'total_time': time_s[-1] - time_s[0] if len(time_s) > 1 else 0,
        'data_points': len(cwnd),
        'total_losses': len(loss_times)
    }
    
    # Détection des phases TCP
    if len(cwnd) > 1:
        cwnd_diff = np.diff(cwnd)
        stats['slow_start_periods'] = np.sum(cwnd_diff > 1)  # Croissance rapide
        stats['congestion_avoidance_periods'] = np.sum(cwnd_diff == 1)  # Croissance linéaire
        
        # Détection des réductions de fenêtre (pertes)
        cwnd_reductions = np.where(cwnd_diff < 0)[0]
        stats['packet_loss_events'] = len(cwnd_reductions)
    else:
        stats['slow_start_periods'] = 0
        stats['congestion_avoidance_periods'] = 0
        stats['packet_loss_events'] = 0
        cwnd_reductions = []
    
    # Calcul de l'efficacité
    stats['efficiency'] = np.mean(cwnd) / stats['max_cwnd'] * 100 if stats['max_cwnd'] > 0 else 0
    
    # Taux de perte
    stats['loss_rate'] = (len(loss_times) / len(time_ms)) * 1000 if len(time_ms) > 0 else 0
    
    return stats, cwnd_reductions

def create_professional_plot(time_ms, cwnd, stats, loss_events, loss_times):
    """Crée un graphique professionnel complet"""
    time_s = time_ms / 1000.0  # Conversion en secondes
    
    # Création de la figure avec subplots
    fig = plt.figure(figsize=(16, 12))
    
    # Subplot principal - Évolution de la fenêtre
    ax1 = plt.subplot2grid((3, 2), (0, 0), colspan=2, rowspan=2)
    
    # Courbe principale
    ax1.plot(time_s, cwnd, color='#2E86AB', linewidth=2.5, 
                   label='Fenêtre de Congestion (cwnd)', alpha=0.8)
    
    # Marquage des pertes de paquets depuis le fichier de pertes
    if len(loss_times) > 0 and len(time_s) > 0:
        loss_times_s = loss_times / 1000.0
        # Trouver les valeurs CWND correspondantes aux temps de perte
        loss_values = []
        for loss_time in loss_times_s:
            # Trouver l'index le plus proche
            idx = np.argmin(np.abs(time_s - loss_time))
            if idx < len(cwnd):
                loss_values.append(cwnd[idx])
        
        ax1.scatter(loss_times_s, loss_values, color='#A23B72', s=80, 
                   zorder=5, label=f'Pertes de paquets ({len(loss_times)})', edgecolors='black', linewidth=1)
    
    # Zones colorées pour les phases TCP
    if stats and 'max_cwnd' in stats and stats['max_cwnd'] > 0:
        ax1.axhspan(0, stats['max_cwnd'] * 0.3, alpha=0.1, color='green', label='Slow Start')
        ax1.axhspan(stats['max_cwnd'] * 0.3, stats['max_cwnd'], alpha=0.1, color='orange', 
                   label='Congestion Avoidance')
    
    # Configuration de l'axe principal
    ax1.set_xlabel('Temps (s)', fontweight='bold')
    ax1.set_ylabel('Fenêtre de Congestion (paquets)', fontweight='bold')
    ax1.set_title('Évolution de la Fenêtre de Congestion TCP\nAnalyse du Comportement et des Pertes', 
                 fontweight='bold', pad=20)
    
    # Grille et échelle
    ax1.grid(True, alpha=0.3, linestyle='--')
    ax1.set_ylim(bottom=0)
    
    # Légende
    ax1.legend(loc='upper right', framealpha=0.9, shadow=True)
    
    # Subplot 2 - Distribution de la fenêtre
    ax2 = plt.subplot2grid((3, 2), (2, 0))
    if len(cwnd) > 0:
        n, bins, patches = ax2.hist(cwnd, bins=20, color='#4CB5AE', alpha=0.7, edgecolor='black')
        ax2.set_xlabel('Fenêtre de Congestion', fontweight='bold')
        ax2.set_ylabel('Fréquence', fontweight='bold')
        ax2.set_title('Distribution de la Fenêtre', fontweight='bold')
        ax2.grid(True, alpha=0.3)
    else:
        ax2.text(0.5, 0.5, 'Aucune donnée', transform=ax2.transAxes, 
                ha='center', va='center', fontsize=14)
        ax2.set_title('Distribution de la Fenêtre', fontweight='bold')
    
    # Subplot 3 - Statistiques textuelles
    ax3 = plt.subplot2grid((3, 2), (2, 1))
    ax3.axis('off')
    
    # Texte des statistiques
    if stats:
        stats_text = (
            f"STATISTIQUES TCP\n"
            f"{'='*30}\n"
            f"Fenêtre max: {stats['max_cwnd']:.1f} paquets\n"
            f"Fenêtre moyenne: {stats['mean_cwnd']:.1f} paquets\n"
            f"Écart-type: {stats['std_cwnd']:.1f} paquets\n"
            f"Pertes totales: {stats['total_losses']}\n"
            f"Événements de perte: {stats['packet_loss_events']}\n"
            f"Taux de perte: {stats['loss_rate']:.1f}‰\n"
            f"Périodes Slow Start: {stats['slow_start_periods']}\n"
            f"Périodes Cong. Avoid: {stats['congestion_avoidance_periods']}\n"
            f"Efficacité: {stats['efficiency']:.1f}%\n"
            f"Durée totale: {stats['total_time']:.1f}s\n"
            f"Points de données: {stats['data_points']}"
        )
    else:
        stats_text = "Aucune donnée TCP disponible\nUtilisation de données simulées"
    
    ax3.text(0.1, 0.9, stats_text, transform=ax3.transAxes, fontfamily='monospace',
            verticalalignment='top', fontsize=11, bbox=dict(boxstyle="round,pad=0.5", 
            facecolor='lightblue', alpha=0.7))
    
    # Ajustement de l'espacement
    plt.tight_layout()
    plt.subplots_adjust(hspace=0.4, wspace=0.3)
    
    return fig

def create_trend_analysis(time_ms, cwnd, loss_times):
    """Analyse des tendances et courbes de tendance"""
    if len(time_ms) == 0:
        return None
    
    time_s = time_ms / 1000.0
    
    fig, (ax1, ax2) = plt.subplots(2, 1, figsize=(14, 10))
    
    # Courbe originale avec tendance polynomiale
    ax1.plot(time_s, cwnd, 'b-', alpha=0.7, linewidth=1, label='Fenêtre réelle')
    
    # Marquer les pertes
    if len(loss_times) > 0:
        loss_times_s = loss_times / 1000.0
        loss_values = []
        for loss_time in loss_times_s:
            idx = np.argmin(np.abs(time_s - loss_time))
            if idx < len(cwnd):
                loss_values.append(cwnd[idx])
        ax1.scatter(loss_times_s, loss_values, color='red', s=30, label='Pertes')
    
    # Courbe de tendance polynomiale
    if len(time_s) > 10:
        z = np.polyfit(time_s, cwnd, 3)
        p = np.poly1d(z)
        ax1.plot(time_s, p(time_s), 'r-', linewidth=2, label='Tendance (poly deg 3)')
    
    ax1.set_xlabel('Temps (s)')
    ax1.set_ylabel('Fenêtre de Congestion')
    ax1.set_title('Analyse de Tendance - Fenêtre TCP avec Pertes')
    ax1.legend()
    ax1.grid(True, alpha=0.3)
    
    # Fenêtre mobile moyenne
    window_size = min(50, len(cwnd) // 10)
    if window_size > 1:
        moving_avg = np.convolve(cwnd, np.ones(window_size)/window_size, mode='valid')
        ax2.plot(time_s[window_size-1:], moving_avg, 'g-', linewidth=2, 
                label=f'Moyenne mobile (n={window_size})')
        ax2.plot(time_s, cwnd, 'b-', alpha=0.3, linewidth=1, label='Fenêtre réelle')
        ax2.set_xlabel('Temps (s)')
        ax2.set_ylabel('Fenêtre de Congestion')
        ax2.set_title('Moyenne Mobile')
        ax2.legend()
        ax2.grid(True, alpha=0.3)
    else:
        ax2.text(0.5, 0.5, 'Données insuffisantes\npour moyenne mobile', 
                transform=ax2.transAxes, ha='center', va='center')
        ax2.set_title('Moyenne Mobile')
    
    plt.tight_layout()
    return fig

def main():
    """Fonction principale"""
    print(" Analyse professionnelle du comportement TCP")
    print("=" * 50)
    
    # Chargement des données
    time_ms, cwnd, loss_times = load_and_analyze_data()
    
    if time_ms is None or len(time_ms) == 0:
        print("❌ Impossible de charger ou générer des données TCP")
        return
    
    # Analyse des données
    stats, loss_events = analyze_tcp_behavior(time_ms, cwnd, loss_times)
    
    # Affichage des statistiques en console
    print("\n📊 STATISTIQUES RÉCAPITULATIVES:")
    if stats:
        print(f"   • Fenêtre max: {stats['max_cwnd']:.1f} paquets")
        print(f"   • Fenêtre moyenne: {stats['mean_cwnd']:.1f} paquets")
        print(f"   • Pertes totales: {stats['total_losses']} paquets")
        print(f"   • Événements de perte: {stats['packet_loss_events']}")
        print(f"   • Taux de perte: {stats['loss_rate']:.1f}‰")
        print(f"   • Efficacité: {stats['efficiency']:.1f}%")
        print(f"   • Durée d'analyse: {stats['total_time']:.1f}s")
    else:
        print("   • Données simulées utilisées")
    
    # Création des graphiques
    print("\n🎨 Génération des visualisations...")
    
    # Graphique principal
    fig1 = create_professional_plot(time_ms, cwnd, stats, loss_events, loss_times)
    if fig1:
        plt.savefig('tcp_congestion_analysis.png', dpi=300, bbox_inches='tight', facecolor='white')
        print("✓ tcp_congestion_analysis.png sauvegardé")
    
    # Analyse de tendance
    fig2 = create_trend_analysis(time_ms, cwnd, loss_times)
    if fig2:
        plt.savefig('tcp_trend_analysis.png', dpi=300, bbox_inches='tight', facecolor='white')
        print("✓ tcp_trend_analysis.png sauvegardé")
    
    print("\n✅ Analyse terminée!")
    print("\n📁 Fichiers utilisés:")
    print("   • tcp_cwnd_data.csv - Données de fenêtre TCP")
    print("   • tcp_loss_data.csv - Données des pertes")
    
    print("\n💡 INTERPRÉTATION:")
    print("Les pertes de paquets (points roses) déclenchent le mécanisme")
    print("de contrôle de congestion TCP, visible par les réductions")
    print("de la fenêtre de congestion.")
    
    # Afficher les graphiques
    plt.show()

if __name__ == "__main__":
    main()
