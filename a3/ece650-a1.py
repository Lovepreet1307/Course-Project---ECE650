import sys
import math
import re
from typing import List, Set, Dict, Optional, Tuple
from dataclasses import dataclass

@dataclass
class Point:
    x: float
    y: float
    
    def __str__(self) -> str:
        return f'({self.x},{self.y})'
    
    def __eq__(self, other) -> bool:
        if not isinstance(other, Point):
            return False
        return self.x == other.x and self.y == other.y
    
    def __hash__(self) -> int:
        return hash((self.x, self.y))

class LineSegment:
    def __init__(self, start: Point, end: Point):
        self.start = start
        self.end = end
    
    def __str__(self) -> str:
        return f'{self.start}-->{self.end}'
    
    def contains_point(self, point: Point, tolerance: float = 1e-10) -> bool:
        return is_point_on_segment(self.start, point, self.end, tolerance)

class Street:
    def __init__(self, name: str, segments: List[LineSegment]):
        self.name = name.lower()
        self.segments = segments
    
    def __str__(self) -> str:
        return f'{self.name}-->{self.segments}'

class Graph:
    def __init__(self):
        self.vertices: Set[Point] = set()
        self.edges: Set[Tuple[Point, Point]] = set()
    
    def add_vertex(self, point: Point) -> None:
        self.vertices.add(point)
    
    def add_edge(self, point1: Point, point2: Point) -> None:
        if point1 != point2:
            self.edges.add((point1, point2))

def calculate_distance(p1: Point, p2: Point) -> float:
    return math.sqrt((p2.x - p1.x)**2 + (p2.y - p1.y)**2)

def cross_product(p1: Point, p2: Point, p3: Point) -> float:
    return (p2.y - p1.y) * (p3.x - p2.x) - (p2.x - p1.x) * (p3.y - p2.y)

def is_point_on_segment(start: Point, point: Point, end: Point, tolerance: float = 1e-10) -> bool:
    min_x = min(start.x, end.x) - tolerance
    max_x = max(start.x, end.x) + tolerance
    min_y = min(start.y, end.y) - tolerance
    max_y = max(start.y, end.y) + tolerance
    
    return (min_x <= point.x <= max_x and 
            min_y <= point.y <= max_y and 
            abs(cross_product(start, point, end)) <= tolerance)

def find_intersection(seg1: LineSegment, seg2: LineSegment) -> Optional[Point]:
    x1, y1 = seg1.start.x, seg1.start.y
    x2, y2 = seg1.end.x, seg1.end.y
    x3, y3 = seg2.start.x, seg2.start.y
    x4, y4 = seg2.end.x, seg2.end.y
    
    det = (x1 - x2) * (y3 - y4) - (y1 - y2) * (x3 - x4)
    
    if abs(det) < 1e-10:  # Lines are parallel
        if x2 == x3 and y2 == y3:
            return Point(x2, y2)
        return None
    
    x_num = ((x1*y2 - y1*x2)*(x3 - x4) - (x1 - x2)*(x3*y4 - y3*x4))
    x_intersect = x_num / det
    
    y_num = (x1*y2 - y1*x2)*(y3 - y4) - (y1 - y2)*(x3*y4 - y3*x4)
    y_intersect = y_num / det
    
    # Check if intersection point lies on both segments
    point = Point(x_intersect, y_intersect)
    if (is_point_on_segment(seg1.start, point, seg1.end) and 
        is_point_on_segment(seg2.start, point, seg2.end)):
        return point
    return None

def find_intersections(streets: List[Street]) -> Set[Point]:
    intersections = set()
    
    for i, street1 in enumerate(streets):
        for street2 in streets[i+1:]:
            for seg1 in street1.segments:
                for seg2 in street2.segments:
                    intersection = find_intersection(seg1, seg2)
                    if intersection:
                        intersections.add(intersection)
    
    return intersections

def generate_graph(streets: List[Street], intersections: Set[Point]) -> Graph:
    graph = Graph()
    
    # Add all intersection points as vertices
    for point in intersections:
        graph.add_vertex(point)
    
    for street in streets:
        for segment in street.segments:
            # Find all intersections on this segment
            segment_intersections = sorted(
                [point for point in intersections if segment.contains_point(point)],
                key=lambda p: calculate_distance(segment.start, p)
            )
            
            # Add edges between consecutive intersections
            if len(segment_intersections) >= 2:
                for i in range(len(segment_intersections) - 1):
                    graph.add_edge(segment_intersections[i], segment_intersections[i + 1])
            elif len(segment_intersections) == 1:
                # Connect single intersection to segment endpoints if no other intersections
                if segment_intersections[0] != segment.start:
                    graph.add_vertex(segment.start)
                    graph.add_edge(segment_intersections[0], segment.start)
                if segment_intersections[0] != segment.end:
                    graph.add_vertex(segment.end)
                    graph.add_edge(segment_intersections[0], segment.end)
    
    return graph

def print_graph(graph: Graph) -> None:
    if len(graph.vertices) < 2:
        return
    
    # Create vertex mapping
    vertex_map = {vertex: idx + 1 for idx, vertex in enumerate(graph.vertices)}
    
    # Remove duplicate edges (including reversed edges)
    unique_edges = {tuple(sorted((v1, v2), key=lambda p: (p.x, p.y))) 
                   for v1, v2 in graph.edges}
    
    if not unique_edges:
        return
    
    # Format output
    edges_str = ','.join(f'<{vertex_map[v1]},{vertex_map[v2]}>' 
                        for v1, v2 in unique_edges)
    
    print(f'V {len(graph.vertices)}\nE {{{edges_str}}}')
    sys.stdout.flush()

class StreetCommand:
    ADD = 'add'
    MODIFY = 'mod'
    REMOVE = 'rm'
    GENERATE = 'gg'
    
    COMMAND_PATTERN = (
        r'\s*(add|mod|rm|gg)\s+'
        r'((?:\s*"[a-zA-Z ]+?"\s*?)?)'
        r'((?:\(\s*-?\s*[0-9]+?\s*,\s*-?\s*[0-9]+?\s*\)\s*?)*)?\s*$'
    )
    
    @staticmethod
    def parse_command(line: str) -> Optional[Tuple[str, str, str]]:
        """Parse command line input."""
        match = re.match(StreetCommand.COMMAND_PATTERN, line)
        return match.groups() if match else None
    
    @staticmethod
    def parse_coordinates(coord_str: str) -> List[str]:
        """Extract coordinate pairs from string."""
        return re.findall(r'\(\s*-?\s*[0-9]+?\s*,\s*-?\s*[0-9]+?\s*\)', coord_str)
    
    @staticmethod
    def create_segments(coordinates: List[str]) -> List[LineSegment]:
        segments = []
        for i in range(len(coordinates) - 1):
            p1_coords = coordinates[i].strip('()').split(',')
            p2_coords = coordinates[i + 1].strip('()').split(',')
            
            start = Point(float(p1_coords[0]), float(p1_coords[1]))
            end = Point(float(p2_coords[0]), float(p2_coords[1]))
            
            segments.append(LineSegment(start, end))
        return segments

class StreetManager:
    def __init__(self):
        self.streets: List[Street] = []
    
    def add_street(self, name: str, coordinates: List[str]) -> bool:
        if not name or name.isspace():
            print("Error: Street name required", file=sys.stderr)
            return False
        
        name = name.lower()
        if any(street.name == name for street in self.streets):
            print(f"Error: Street '{name}' already exists", file=sys.stderr)
            return False
        
        if len(coordinates) < 2:
            print("Error: Minimum 2 coordinates required", file=sys.stderr)
            return False
        
        segments = StreetCommand.create_segments(coordinates)
        self.streets.append(Street(name, segments))
        return True
    
    def modify_street(self, name: str, coordinates: List[str]) -> bool:
        if not name or name.isspace():
            print("Error: Street name required", file=sys.stderr)
            return False
        
        if len(coordinates) < 2:
            print("Error: Minimum 2 coordinates required", file=sys.stderr)
            return False
        
        name = name.lower()
        for i, street in enumerate(self.streets):
            if street.name == name:
                segments = StreetCommand.create_segments(coordinates)
                self.streets[i] = Street(name, segments)
                return True
        
        print("Error: Street does not exist", file=sys.stderr)
        return False
    
    def remove_street(self, name: str) -> bool:
        if not name or name.isspace():
            print("Error: Street name required", file=sys.stderr)
            return False
        
        name = name.lower()
        for i, street in enumerate(self.streets):
            if street.name == name:
                self.streets.pop(i)
                return True
        
        print("Error: Street does not exist", file=sys.stderr)
        return False
    
    def generate_graph(self) -> None:
        intersections = find_intersections(self.streets)
        graph = generate_graph(self.streets, intersections)
        print_graph(graph)

def main():
    manager = StreetManager()
    
    while True:
        try:
            line = sys.stdin.readline()
            if not line:
                break
            
            # Parse command
            parsed = StreetCommand.parse_command(line)
            if not parsed:
                print("Error: Invalid command format", file=sys.stderr)
                continue
            
            operation, raw_name, raw_coords = parsed
            
            # Validate input format
            if ')(' in raw_coords or '"(' in line:
                print("Error: Invalid coordinate format", file=sys.stderr)
                continue
            
            # Extract street name if present
            street_name = raw_name.split('"')[1] if raw_name else ""
            
            # Parse coordinates if present
            coordinates = StreetCommand.parse_coordinates(raw_coords)
            
            # Process commands
            if operation == StreetCommand.ADD:
                manager.add_street(street_name, coordinates)
            elif operation == StreetCommand.MODIFY:
                manager.modify_street(street_name, coordinates)
            elif operation == StreetCommand.REMOVE:
                if raw_coords:
                    print("Error: Remove command does not accept coordinates", file=sys.stderr)
                    continue
                manager.remove_street(street_name)
            elif operation == StreetCommand.GENERATE:
                if raw_name or raw_coords:
                    print("Error: Generate command takes no arguments", file=sys.stderr)
                    continue
                manager.generate_graph()
                
        except Exception as e:
            print(f"Error: Invalid input format - {str(e)}", file=sys.stderr)
            continue
    
    sys.exit(0)

if __name__ == "__main__":
    main()