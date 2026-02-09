import re
import sys


def parse_input_file(input_str):
    lines = [line.strip() for line in input_str.strip().splitlines() if line.strip()]
    width, height = map(int, lines[0].split())
    assert width > 0 and height > 0, "Invalid chip size"
    blockages = []
    nets = []
    i = 1
    assert lines[i].startswith("#blockages"), "Missing blockages header"
    n_blockages = int(lines[i].split()[1])
    for j in range(n_blockages):
        x1, y1, x2, y2 = map(int, lines[i + 1 + j].split())
        # 檢查 block 不可超出範圍且為 1-indexed
        if not (1 <= x1 <= x2 <= width and 1 <= y1 <= y2 <= height):
            raise ValueError(
                f"Blockage #{j+1} ({x1}, {y1}, {x2}, {y2}) 超出範圍 1~{width}, 1~{height}"
            )
        blockages.append((x1, y1, x2, y2))
    i += n_blockages + 1
    assert lines[i].startswith("#interconnections"), "Missing interconnections header"
    n_nets = int(lines[i].split()[1])
    for j in range(n_nets):
        x1, y1, x2, y2 = map(int, lines[i + 1 + j].split())
        # 檢查 net 端點是否在 1-indexed 範圍
        if not (1 <= x1 <= width and 1 <= y1 <= height and 1 <= x2 <= width and 1 <= y2 <= height):
            raise ValueError(
                f"Net #{j+1} 的端點 ({x1}, {y1}), ({x2}, {y2}) 超出範圍 1~{width}, 1~{height}"
            )
        nets.append(((x1, y1), (x2, y2)))
    return width, height, blockages, nets


def parse_output_file(output_str, n_nets):
    lines = [line.strip() for line in output_str.strip().splitlines() if line.strip()]
    # Parse statistics in header
    stats = {}
    header_fields = [
        ("#interconnections routed", int),
        ("Total interconnection length", int),
        ("The longest interconnection", str),
        ("Total number of bends", int),
    ]
    stat_idx = 0
    for label, typ in header_fields:
        if stat_idx >= len(lines) or not lines[stat_idx].startswith(label):
            raise ValueError(f"Missing output header field: {label}")
        if label == "The longest interconnection":
            # e.g. "The longest interconnection = 2; length = 18"
            m = re.match(r"The longest interconnection = (\d+); length = (\d+)", lines[stat_idx])
            if not m:
                raise ValueError("Malformed longest interconnection stat line.")
            stats["longest_idx"] = int(m.group(1))
            stats["longest_len"] = int(m.group(2))
        else:
            stats[label] = typ(lines[stat_idx].split("=")[1].strip())
        stat_idx += 1

    # Parse each net's output
    net_results = []
    net_idx = 0
    i = stat_idx
    while net_idx < n_nets:
        if i >= len(lines):
            raise ValueError(f"Not enough output lines for net {net_idx+1}")
        if lines[i].startswith("Interconnection"):
            if "fails" in lines[i]:
                net_results.append({"path": None, "length": None, "bends": None, "fail": True})
                net_idx += 1
                i += 1
            else:
                m = re.match(r"Interconnection (\d+): length = (\d+), #bends = (\d+)", lines[i])
                if not m or int(m.group(1)) != net_idx + 1:
                    raise ValueError(f"Output net index/order error at line: {lines[i]}")
                length = int(m.group(2))
                bends = int(m.group(3))
                # Next line is path
                path_line = lines[i + 1]
                points = re.findall(r"\((\d+),\s*(\d+)\)", path_line)
                path = [(int(x), int(y)) for x, y in points]
                net_results.append({"path": path, "length": length, "bends": bends, "fail": False})
                net_idx += 1
                i += 2
        else:
            i += 1
    return stats, net_results


def point_in_blockages(point, blockages):
    x, y = point
    for x1, y1, x2, y2 in blockages:
        if x1 <= x <= x2 and y1 <= y <= y2:
            return True
    return False


def count_bends(path):
    if not path or len(path) < 2:
        return 0
    bends = 0
    prev_dir = None
    for i in range(1, len(path)):
        dx = path[i][0] - path[i - 1][0]
        dy = path[i][1] - path[i - 1][1]
        direction = (dx, dy)
        if prev_dir is not None and direction != prev_dir:
            bends += 1
        prev_dir = direction
    return bends


def validate_routing(input_str, output_str, verbose=True):
    width, height, blockages, nets = parse_input_file(input_str)
    n_nets = len(nets)
    stats, net_results = parse_output_file(output_str, n_nets)

    used_cells = set()
    success = True

    # For statistics calculation
    routed_count = 0
    total_len = 0
    total_bends = 0
    longest_len = 0
    longest_idx = 0

    for netid, ((sx, sy), (ex, ey)) in enumerate(nets):
        entry = net_results[netid]
        path = entry["path"]
        out_len = entry["length"]
        out_bends = entry["bends"]
        out_fail = entry["fail"]

        if path is None:
            if not out_fail:
                print(f"Net {netid+1}: Marked not failed, but no path.")
                success = False
            continue  # fail case

        # 1. Path length and bends match output
        computed_length = len(path) - 1
        computed_bends = count_bends(path)
        if computed_length != out_len:
            print(f"Net {netid+1}: Output length {out_len}, computed {computed_length}")
            success = False
        if computed_bends != out_bends:
            print(f"Net {netid+1}: Output bends {out_bends}, computed {computed_bends}")
            success = False

        # 2. Check start/end
        if path[0] != (sx, sy):
            print(f"Net {netid+1}: Start mismatch: expect {sx, sy}, got {path[0]}")
            success = False
        if path[-1] != (ex, ey):
            print(f"Net {netid+1}: End mismatch: expect {ex, ey}, got {path[-1]}")
            success = False

        # 3. Check bounds, blockage, overlap
        for px, py in path:
            if not (1 <= px <= width and 1 <= py <= height):
                print(f"Net {netid+1}: Point {px, py} out of chip bounds")
                success = False
            if point_in_blockages((px, py), blockages):
                print(f"Net {netid+1}: Point {px, py} is inside a blockage")
                success = False
            if (px, py) in used_cells:
                print(f"Net {netid+1}: Point {px, py} overlaps with another net")
                success = False
            used_cells.add((px, py))

        # 4. Path continuity
        for i in range(1, len(path)):
            dx = abs(path[i][0] - path[i - 1][0])
            dy = abs(path[i][1] - path[i - 1][1])
            if dx + dy != 1:
                print(f"Net {netid+1}: Not continuous between {path[i-1]} and {path[i]}")
                success = False

        # 5. Path no repeat
        if len(path) != len(set(path)):
            print(f"Net {netid+1}: Path has repeated points (cycle)")
            success = False

        # For statistics
        routed_count += 1
        total_len += computed_length
        total_bends += computed_bends
        if computed_length > longest_len:
            longest_len = computed_length
            longest_idx = netid + 1

    # Check statistics
    if stats["#interconnections routed"] != routed_count:
        print(
            f"Routed count mismatch: output={stats['#interconnections routed']}, computed={routed_count}"
        )
        success = False
    if stats["Total interconnection length"] != total_len:
        print(
            f"Total length mismatch: output={stats['Total interconnection length']}, computed={total_len}"
        )
        success = False
    if stats["longest_idx"] != (longest_idx if routed_count > 0 else 0) or stats["longest_len"] != (
        longest_len if routed_count > 0 else 0
    ):
        print(
            f"Longest interconnection mismatch: output=({stats['longest_idx']}, {stats['longest_len']}), computed=({longest_idx}, {longest_len})"
        )
        success = False
    if stats["Total number of bends"] != total_bends:
        print(
            f"Total bends mismatch: output={stats['Total number of bends']}, computed={total_bends}"
        )
        success = False
    # Check failed nets
    if len(net_results) != n_nets:
        print(f"Net count mismatch: input={n_nets}, output={len(net_results)}")
        success = False
    for netid, entry in enumerate(net_results):
        if entry["path"] is None and not entry["fail"]:
            print(f"Net {netid+1}: No path but not marked failed")
            success = False
        if entry["path"] is not None and entry["fail"]:
            print(f"Net {netid+1}: Path present but marked failed")
            success = False

    if success:
        if verbose:
            print("All nets and statistics are routed and reported legally.")
    else:
        if verbose:
            print("Output is NOT fully legal, see messages above.")

    # 回傳格式：(合法與否, routed, total_len, longest_len, total_bends)
    return success, routed_count, total_len, longest_len, total_bends


if __name__ == "__main__":
    try:
        if len(sys.argv) != 3:
            print("Usage: evaluator [input_file] [output_file]")
            sys.exit(1)
        with open(sys.argv[1]) as f:
            input_str = f.read()
        with open(sys.argv[2]) as f:
            output_str = f.read()
        ok, routed, total_len, longest, total_bends = validate_routing(
            input_str, output_str, verbose=True
        )
    except Exception as e:
        print(str(e))
        sys.exit(1)
