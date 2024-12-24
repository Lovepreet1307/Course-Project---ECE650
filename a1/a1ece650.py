import re
import sys
import math

class Coordinate(object):
    def __init__ (self, x, y):
        self.x = float(x)
        self.y = float(y)
    def __str__ (self):
        return '(' + str(self.x) + ',' + str(self.y) + ')'
    
    def __eq__(self, other):
        return hasattr(other, 'x') and hasattr(other, 'y') and self.x == other.x and self.y == other.y

    def __hash__(self):
         return hash((self.x, self.y))
    
class Segment(object):
    def __init__ (self, src, dst):
        self.src = src
        self.dst = dst

    def __str__(self):
        return str(self.src) + '-->' + str(self.dst)

def calculate_determinant(x1, y1, x2, y2, x3, y3, x4, y4):
    return (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)

def calculate_intersection_coordinates(x1, y1, x2, y2, x3, y3, x4, y4, det):
    xnum = ((x1 * y2 - y1 * x2) * (x3 - x4) - (x1 - x2) * (x3 * y4 - y3 * x4))
    xden = det
    xcoor = xnum / xden

    ynum = (x1 * y2 - y1 * x2) * (y3 - y4) - (y1 - y2) * (x3 * y4 - y3 * x4)
    yden = det
    ycoor = ynum / yden

    return xcoor, ycoor

def intersect(line1, line2):
    a1, b1 = line1.src.x, line1.src.y
    a2, b2 = line1.dst.x, line1.dst.y
    a3, b3 = line2.src.x, line2.src.y
    a4, b4 = line2.dst.x, line2.dst.y

    e_d = calculate_determinant(a1, b1, a2, b2, a3, b3, a4, b4)

    if e_d == 0:
        if a2 == a3 and b2 == b3:
            return Coordinate(a2, b2)
        return None

    coordinate_x, coordinate_y = calculate_intersection_coordinates(a1, b1, a2, b2, a3, b3, a4, b4, e_d)

    within_line1=check_if_lies_the_point_within_other_points(line1.src.x, coordinate_x, line1.dst.x) and check_if_lies_the_point_within_other_points(line1.src.y, coordinate_y, line1.dst.y)
    within_line2=check_if_lies_the_point_within_other_points(line2.src.x, coordinate_x, line2.dst.x) and check_if_lies_the_point_within_other_points(line2.src.y, coordinate_y, line2.dst.y)
   
    if within_line1 and within_line2:
        return Coordinate(coordinate_x, coordinate_y)

    return None

def check_if_lies_the_point_within_other_points(p, q, r):
    return (p <= q <= r) or (r <= q <= p)

class Road:
    def __init__(self, name, list_of_lines):
        self.name=name
        self.list_of_lines=list_of_lines

    def __str__(self):
        lines_str = ', '.join(map(str, self.list_of_lines))
        return f"{self.name} --> {lines_str}"
    
def prod_cross(point1, point2, point3):
    diff_y1 = point2.y - point1.y
    diff_x2 = point3.x - point2.x
    diff_x1 = point2.x - point1.x
    diff_y2 = point3.y - point2.y
    return diff_y1 * diff_x2 - diff_x1 * diff_y2
    
def partial_overlap(l1, l2):
    if prod_cross(l1.src, l1.dst, l2.src) == 0 and prod_cross(l1.src, l1.dst, l2.dst) == 0:
        if check_if_lies_the_point_within_other_points(l1.src.x, l2.src.x, l1.dst.x) or check_if_lies_the_point_within_other_points(l1.src.x, l2.dst.x, l1.dst.x) or \
           check_if_lies_the_point_within_other_points(l2.src.x, l1.src.x, l2.dst.x) or check_if_lies_the_point_within_other_points(l2.src.x, l1.dst.x, l2.dst.x):
            return True
    return False

def to_find_segment(start, mid, end, threshold=1e-10):
    
    diff_x = abs(start.x - end.x)
    diff_y = abs(start.y - end.y)

    in_x_range = min(start.x, end.x) - threshold <= mid.x <= max(start.x, end.x) + threshold
    in_y_range = min(start.y, end.y) - threshold <= mid.y <= max(start.y, end.y) + threshold

    is_collinear = abs((mid.x - start.x) * (end.y - start.y) - (end.x - start.x) * (mid.y - start.y)) <= threshold

    return in_x_range and in_y_range and is_collinear

def to_find_intersection(group_of_st):
    inter_points = set()

    for main_street in group_of_st:
        for main_line in main_street.list_of_lines:
            lines_to_check = [line for street in group_of_st if street != main_street for line in street.list_of_lines]
            
            for other_line in lines_to_check:
                if partial_overlap(main_line, other_line):
                    points_to_add = {main_line.src, main_line.dst, other_line.src, other_line.dst}
                    inter_points.update(points_to_add - {None})
                else:
                    intersection = intersect(main_line, other_line)
                    if intersection:
                        inter_points.add(intersection)
    
    return inter_points

def compute_dist(p1, p2):
    x_diff = p2.x - p1.x
    y_diff = p2.y - p1.y
    dist_squared = x_diff**2 + y_diff**2
    return dist_squared**0.5

def normalize_coordinate(value):
    if value == -0.0:
        return 0.0
    return value

def build_undirected(road_segments, cross_points):

    node_set = set()  
    connection_paths = [] 

    for point in cross_points:
        add_vertex(node_set, point)

    path_points = set()

    for road in road_segments:
        for segment in road.list_of_lines:
            path_points.add(segment.src)
            path_points.add(segment.dst)

    nearest_src_cross = {}
    nearest_dst_cross = {}

    for cross_point in cross_points:
        for road in road_segments:
            road_lines = road.list_of_lines
            for segment in road_lines:

                if to_find_segment(segment.src, cross_point, segment.dst):
                    if len(cross_points) == 1:
                        if cross_point != segment.src:
                            add_edge(connection_paths, node_set, cross_point, segment.src)
                        if cross_point != segment.dst:
                            add_edge(connection_paths, node_set, cross_point, segment.dst)
                        add_vertex(node_set, segment.src)
                        add_vertex(node_set, segment.dst)
                        continue

                    count = sum(1 for other_point in cross_points if other_point != cross_point and to_find_segment(segment.src, other_point, segment.dst))

                    if count == 0:
                        if cross_point != segment.src:
                            add_edge(connection_paths, node_set, cross_point, segment.src)
                        if cross_point != segment.dst:
                            add_edge(connection_paths, node_set, cross_point, segment.dst)
                        add_vertex(node_set, segment.src)
                        add_vertex(node_set, segment.dst)

                    for another_cross in cross_points:
                        if another_cross == cross_point:
                            continue

                        if to_find_segment(segment.src, another_cross, segment.dst):
                            if to_find_segment(cross_point, another_cross, segment.src):
                                segment_edge = Segment(cross_point, another_cross)
                                nearest_src_cross[compute_dist(cross_point, another_cross)] = segment_edge
                            else:
                                found_other_cross = any(
                                    to_find_segment(cross_point, other_point, segment.src)
                                    for other_point in cross_points
                                    if other_point != cross_point
                                )

                                if not found_other_cross and cross_point != segment.src:
                                    add_edge(connection_paths, node_set, cross_point, segment.src)
                                    add_vertex(node_set, segment.src)

                            if to_find_segment(cross_point, another_cross, segment.dst):
                                segment_edge = Segment(cross_point, another_cross)
                                nearest_dst_cross[compute_dist(cross_point, another_cross)] = segment_edge
                            else:
                                found_other_cross = any(
                                    to_find_segment(cross_point, other_point, segment.dst)
                                    for other_point in cross_points
                                    if other_point != cross_point
                                )

                                if not found_other_cross and cross_point != segment.dst:
                                    add_edge(connection_paths, node_set, cross_point, segment.dst)
                                    add_vertex(node_set, segment.dst)

                if nearest_src_cross:
                    min_src_key = min(nearest_src_cross.keys())
                    min_src_val = nearest_src_cross[min_src_key]
                    if min_src_val.src != min_src_val.dst:
                        add_edge(connection_paths, node_set, min_src_val.src, min_src_val.dst)

                if nearest_dst_cross:
                    min_dst_key = min(nearest_dst_cross.keys())
                    min_dst_val = nearest_dst_cross[min_dst_key]
                    if min_dst_val.src != min_dst_val.dst:
                        add_edge(connection_paths, node_set, min_dst_val.src, min_dst_val.dst)

                nearest_src_cross.clear()
                nearest_dst_cross.clear()

    connection_paths = filter_unique_edges(connection_paths)

    return node_set, connection_paths

def add_vertex(vertices_set, point):
    normalized_point = Coordinate(normalize_coordinate(point.x), normalize_coordinate(point.y))
    vertices_set.add(normalized_point)

def add_edge(edges_list, vertices_set, point1, point2):
    normalized_point1 = Coordinate(normalize_coordinate(point1.x), normalize_coordinate(point1.y))
    normalized_point2 = Coordinate(normalize_coordinate(point2.x), normalize_coordinate(point2.y))
    
    if normalized_point1 != normalized_point2:
        edges_list.append((normalized_point1, normalized_point2))
        vertices_set.add(normalized_point1)
        vertices_set.add(normalized_point2)

def print_vertices(vertex_set):
    vertex_map = {v_idx + 1: v for v_idx, v in enumerate(vertex_set)}
    print("V = {")
    for idx, vertex in vertex_map.items():
        x_coord = normalize_coordinate(round(vertex.x, 2))
        y_coord = normalize_coordinate(round(vertex.y, 2))
        print(f" {idx}:  ({x_coord}, {y_coord})")
    print("}")
    
    return vertex_map

def print_edges(edge_list, vertex_map):
    print('E = {')
    
    def get_vertex_index(vertex):
        for idx, mapped_vertex in vertex_map.items():
            if mapped_vertex == vertex:
                return idx

    for idx, e in enumerate(edge_list):
        start_idx = get_vertex_index(e[0])
        end_idx = get_vertex_index(e[1])
        connector = '' if idx == len(edge_list) - 1 else ','
        print(f"  <{start_idx},{end_idx}>{connector}")
    
    print("}")

def filter_unique_edges(edges_list):
    set_of_edges_uniq = set()
    for i in edges_list:
        tilted_i = (i[1], i[0])
        if i not in set_of_edges_uniq and tilted_i not in set_of_edges_uniq:
            set_of_edges_uniq.add(i)
    return list(set_of_edges_uniq)

def add_street(street_input, raw_coordinates, street_map):
    name_parts = street_input.split('"')
    street_name = name_parts[1].strip().lower()

    if not street_name or street_name.isspace():
        print("Error: No street name provided", file=sys.stderr)
        return

    existing = any(road.name == street_name for road in street_map)

    if existing:
        print(f"Error: Street '{street_name}' already exists!", file=sys.stderr)
        return

    if len(raw_coordinates) < 2:
        print("Error: At least 2 coordinates are required", file=sys.stderr)
        return

    street_segments = []
    
    for coord_pair in zip(raw_coordinates, raw_coordinates[1:]):
        start_point = coord_pair[0].strip("()").split(",")
        end_point = coord_pair[1].strip("()").split(",")
        start = Coordinate(float(start_point[0]), float(start_point[1]))
        end = Coordinate(float(end_point[0]), float(end_point[1]))

        if start == end:
            print(f"Error: Duplicate consecutive coordinates found: {start}", file=sys.stderr)
            return

        street_segments.append(Segment(start, end))

    street_map.append(Road(street_name, street_segments))


def modify_street(street_input, raw_coordinates, street_map):
    name_parts = street_input.split('"')
    street_name = name_parts[1].strip().lower()

    if not street_name or street_name.isspace():
        print("Error: No street name provided", file=sys.stderr)
        return

    if len(raw_coordinates) < 2:
        print("Error: At least 2 coordinates are required", file=sys.stderr)
        return

    existing_street_names = [road.name for road in street_map]

    if street_name not in existing_street_names:
        print(f"Error: Street '{street_name}' does not exist", file=sys.stderr)
        return

    street_map[:] = [road for road in street_map if road.name != street_name]

    updated_segments = []
    
    for coord_pair in zip(raw_coordinates, raw_coordinates[1:]):
        start_point = coord_pair[0].strip("()").split(",")
        end_point = coord_pair[1].strip("()").split(",")
        start = Coordinate(float(start_point[0]), float(start_point[1]))
        end = Coordinate(float(end_point[0]), float(end_point[1]))

        if start == end:
            print(f"Error: Duplicate consecutive coordinates found: {start}", file=sys.stderr)
            return

        updated_segments.append(Segment(start, end))

    street_map.append(Road(street_name, updated_segments))


def remove_street(street_input, street_map):
    if not street_input.strip():
        print("Error: No street name provided", file=sys.stderr)
        return

    name_parts = street_input.split('"')
    street_name = name_parts[1].strip().lower()

    if not street_name or street_name.isspace():
        print("Error: No street name provided", file=sys.stderr)
        return

    existing_street_names = [road.name for road in street_map]

    if street_name not in existing_street_names:
        print(f"Error: Street '{street_name}' does not exist", file=sys.stderr)
        return

    street_map[:] = [road for road in street_map if road.name != street_name]


def generate_graph_and_output(street_map):
    intersections = to_find_intersection(street_map)
    nodes, edges = build_undirected(street_map, intersections)
    vertex_mapping = print_vertices(nodes)
    print_edges(edges, vertex_mapping)

def parse_components(matched):
    command = matched.group(1).strip()
    street_label = matched.group(2).strip() if matched.group(2) else ""
    coordinates_data = matched.group(3).strip() if matched.group(3) else ""
    return command, street_label, coordinates_data

def validate_format(input_line, coordinates_data):
    if '"(' in input_line:
        return "Incorrect format: Invalid placement of parentheses after quotes."
    if ')(' in coordinates_data:
        return "Incorrect format: Missing space between coordinates."
    return None

def main():
    street_map = []
    
    while True:
        input_line = sys.stdin.readline()

        if not input_line.strip():
            break
        
        command_pattern = r'\s*(add|mod|rm|gg)\s+((?:\s*"[a-zA-Z ]+?"\s*?)?)((?:\(\s*-?\s*[0-9]+?\s*,\s*-?\s*[0-9]+?\s*\)\s*?)*)?\s*$'
        matched = re.match(command_pattern, input_line)

        if matched:
            command, street_label, coordinates_data = parse_components(matched)
            
            error_message = validate_format(input_line, coordinates_data)
            if error_message:
                print(f"Error: {error_message}", file=sys.stderr)
                continue
            
            if command in ['add', 'mod']:
                if not street_label:
                    print("Error: Street name is required for 'add' or 'mod'", file=sys.stderr)
                    continue
                
                if len(coordinates_data) == 0:
                    print("Error: Coordinates missing for 'add' or 'mod'", file=sys.stderr)
                    continue
            
            elif command == 'rm' and not street_label:
                    print("Error: Street name is required for 'rm'", file=sys.stderr)
                    continue
            
            elif command == 'gg' and (street_label or coordinates_data):
                   print("Error: No additional input needed for 'gg'", file=sys.stderr)
                   continue
        
            if coordinates_data:
                coordinates = re.findall(r'\(\s*-?\s*[0-9]+?\s*,\s*-?\s*[0-9]+?\s*\)', coordinates_data)
                if not coordinates:
                    print("Error: Invalid coordinates format", file=sys.stderr)
                    continue
        else: 
           print("Error: Incorrect format. Please check your input.", file=sys.stderr)
           continue

        try:
            if command == 'add':
                add_street(street_label, coordinates, street_map)
            elif command == 'mod':
                modify_street(street_label, coordinates, street_map)
            elif command == 'rm':
                remove_street(street_label, street_map)
            elif command == 'gg':
                generate_graph_and_output(street_map)

        except Exception as e:
            print(f"Error: {str(e)}", file=sys.stderr)
            continue
    
    sys.exit(0)

if __name__ == "__main__":
    main()