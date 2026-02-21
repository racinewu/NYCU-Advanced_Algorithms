import matplotlib.pyplot as plt
import matplotlib.patches as patches
import re
from matplotlib.colors import to_rgb
import numpy as np
import os
import sys

def get_contrasting_text_color(bg_color):
    r, g, b = to_rgb(bg_color)
    brightness = r * 0.299 + g * 0.587 + b * 0.114
    return 'black' if brightness > 0.5 else 'white'

def draw_routing(input_file, output_file, save_name, text_scale_factor=0.9):
    if not os.path.exists(input_file):
        raise FileNotFoundError(f"Cannot find input file: {input_file}")

    with open(input_file, 'r') as f:
        lines = [line.strip() for line in f if line.strip() and not line.strip().startswith("//")]

    chipX, chipY = map(int, lines[0].split())
    num_blockages = int(lines[1].split()[1])
    blockages = [tuple(map(int, lines[i+2].split())) for i in range(num_blockages)]
    
    idx = 2 + num_blockages
    num_inters = int(lines[idx].split()[1])
    idx += 1
    interconnections = []
    for i in range(num_inters):
        vals = list(map(int, lines[idx].split()))
        interconnections.append(((vals[0], vals[1]), (vals[2], vals[3])))
        idx += 1

    connections_out = {}
    if os.path.exists(output_file):
        with open(output_file, 'r') as f:
            content = f.read()
        pattern = r"Interconnection\s+(\d+):\s+length\s*=\s*\d+,\s*#bends\s*=\s*\d+\s*((?:\(\d+,\s*\d+\),?\s*)+)"
        for match in re.finditer(pattern, content):
            conn_num = int(match.group(1))
            coords_str = match.group(2)
            pts = re.findall(r"\((\d+),\s*(\d+)\)", coords_str)
            path = [(int(x), int(y)) for x, y in pts]
            if path:
                connections_out[conn_num] = {"path": path}
    else:
        print(f"Warning: Cannot find output file: {output_file}, would only draw start and end points.")

    cell_labels = {}
    for i, (start, end) in enumerate(interconnections, start=1):
        for pos, label_type, is_start in [(start, f's{i}', True), (end, f't{i}', False)]:
            if pos not in cell_labels:
                cell_labels[pos] = []
            cell_labels[pos].append({'label': label_type, 'id': i, 'is_start': is_start})

    num_cols, num_rows = chipX, chipY
    fig_width = 12
    fig, ax = plt.subplots(figsize=(fig_width, fig_width * (num_rows/num_cols)))
    ax.set_xlim(0, num_cols)
    ax.set_ylim(0, num_rows)
    ax.set_aspect('equal')
    ax.set_title("Chip Routing Visualization")
    ax.set_xlabel("X")
    ax.set_ylabel("Y")

    def get_fixed_fs(target_ratio, label_len):
        return (500 / num_cols) * (target_ratio / label_len) * text_scale_factor

    base_fontsize = max(4, 20 - max(num_cols, num_rows) // 6)
    colors = [
        '#E6194B', '#3CB44B', '#FFE119', '#4363D8', '#F58231',
        '#911EB4', '#46F0F0', '#F032E6', '#BCF60C', '#FABEBE',
        '#008080', '#E6BEFF', '#9A6324', '#FFFAC8', '#800000',
        '#AAFFC3', '#808000', '#FFD8B1', '#000075', '#A9A9A9',
        '#FF1493', '#00CED1', '#7FFF00', '#FF4500', '#DA70D6'
    ]

    for bx1, by1, bx2, by2 in blockages:
        lowX, highX = min(bx1, bx2) - 1, max(bx1, bx2)
        lowY, highY = min(by1, by2) - 1, max(by1, by2)
        ax.add_patch(patches.Rectangle((lowX, lowY), highX - lowX, highY - lowY,
                                     facecolor='dimgray', zorder=2))

    routed_cells = set()
    for conn_id, data in connections_out.items():
        color = colors[(conn_id - 1) % len(colors)]
        for (x, y) in data["path"]:
            ax.add_patch(patches.Rectangle((x-1, y-1), 1, 1, facecolor=color, zorder=1))
            routed_cells.add((x, y))

    for pos, labels in cell_labels.items():
        x, y = pos
        successfully_routed = [l for l in labels if l['id'] in connections_out]
        center_node = successfully_routed[0] if successfully_routed else min(labels, key=lambda l: l['id'])
        
        if pos not in routed_cells:
            color = colors[(center_node['id'] - 1) % len(colors)]
            ax.add_patch(patches.Rectangle((x-1, y-1), 1, 1, facecolor=color, zorder=1))

        color = colors[(center_node['id'] - 1) % len(colors)]
        text_color = get_contrasting_text_color(color)
        
        corners = [(0.8, 0.8), (0.8, 0.2), (0.2, 0.2), (0.2, 0.8)]
        corner_idx = 0
        
        for item in labels:
            is_center = (item == center_node)
            label_str = item['label']
            length = len(label_str)
            
            if is_center:
                fs = get_fixed_fs(1.0, length)
                ax.text(x - 0.5, y - 0.5, label_str, color=text_color, 
                        fontsize=fs, fontweight='bold', ha='center', va='center', zorder=5)
            else:
                if corner_idx < 4:
                    fs_corner = get_fixed_fs(0.7, length)
                    dx, dy = corners[corner_idx]
                    ax.text(x - 1 + dx, y - 1 + dy, label_str, color=text_color, 
                            fontsize=fs_corner, fontweight='normal', ha='center', va='center', zorder=6)
                    corner_idx += 1

    grid_width = 0.8
    for i in range(num_cols + 1): ax.axhline(i, color='#CCCCCC', linewidth=grid_width, zorder=3)
    for j in range(num_rows + 1): ax.axvline(j, color='#CCCCCC', linewidth=grid_width, zorder=3)

    ax.set_xticks(np.arange(0.5, num_cols, 1))
    ax.set_yticks(np.arange(0.5, num_rows, 1))
    ax.set_xticklabels(range(1, num_cols + 1), fontsize=base_fontsize * 0.5)
    ax.set_yticklabels(range(1, num_rows + 1), fontsize=base_fontsize * 0.5)

    plt.savefig(save_name, format='svg', bbox_inches='tight', dpi=300)
    plt.show()

if __name__ == "__main__":
    if len(sys.argv) != 4:
        print("Usage: python3 visualizer.py <case>.in <case>.out <case>.svg")
        sys.exit(1)

    input_file = sys.argv[1]
    output_file = sys.argv[2]
    save_name = sys.argv[3]

    try:
        draw_routing(input_file, output_file, save_name)
    except Exception as e:
        print(e)
        sys.exit(1)