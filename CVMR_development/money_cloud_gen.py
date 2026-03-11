'''import numpy as np
import matplotlib.pyplot as plt

# Set random seed for reproducibility
np.random.seed(42)

def generate_building(x0, y0, w, d, h, n_points=100):
    points = []
    for _ in range(n_points):
        face = np.random.choice(['front', 'back', 'left', 'right', 'top', 'bottom'])
        if face == 'front':
            x = np.random.uniform(x0, x0 + w)
            y = y0
            z = np.random.uniform(0, h)
        elif face == 'back':
            x = np.random.uniform(x0, x0 + w)
            y = y0 + d
            z = np.random.uniform(0, h)
        elif face == 'left':
            x = x0
            y = np.random.uniform(y0, y0 + d)
            z = np.random.uniform(0, h)
        elif face == 'right':
            x = x0 + w
            y = np.random.uniform(y0, y0 + d)
            z = np.random.uniform(0, h)
        elif face == 'top':
            x = np.random.uniform(x0, x0 + w)
            y = np.random.uniform(y0, y0 + d)
            z = h
        elif face == 'bottom':
            x = np.random.uniform(x0, x0 + w)
            y = np.random.uniform(y0, y0 + d)
            z = 0
        noise = np.random.normal(0, 0.05, 3)
        point = np.array([x, y, z]) + noise
        points.append(point)
    return np.array(points)

# Generate buildings
building1 = generate_building(0, 0, 10, 10, .1, 150)
building2 = generate_building(15, 0, 10, 8, .12, 150)
building3 = generate_building(30, 0, 5, 5, .08, 100)

# Combine and scale
point_cloud = np.vstack([building1, building2, building3])
scaled_point_cloud = point_cloud * 50

# Plot
fig = plt.figure(figsize=(10, 8))
ax = fig.add_subplot(111, projection='3d')
ax.scatter(scaled_point_cloud[:, 0], scaled_point_cloud[:, 1], scaled_point_cloud[:, 2], s=3)
ax.set_xlabel("X")
ax.set_ylabel("Y")
ax.set_zlabel("Z")
ax.set_title("Sparse 3D Point Cloud of Buildings (Scaled x50)")
plt.tight_layout()
plt.show()

# Optional: Save first 20 points to text
for point in scaled_point_cloud:
    print("{:.5f} {:.5f} {:.5f}".format(*point))
'''

import numpy as np
import matplotlib.pyplot as plt

np.random.seed(42)

def generate_building(x0, y0, w, d, h, n_points_per_face=300):
    """Generate a realistic box-shaped building with points on 5 faces."""
    points = []

    faces = {
        'front': lambda: [np.random.uniform(x0, x0 + w), y0, np.random.uniform(0, h)],
        'back': lambda: [np.random.uniform(x0, x0 + w), y0 + d, np.random.uniform(0, h)],
        'left': lambda: [x0, np.random.uniform(y0, y0 + d), np.random.uniform(0, h)],
        'right': lambda: [x0 + w, np.random.uniform(y0, y0 + d), np.random.uniform(0, h)],
        'top': lambda: [np.random.uniform(x0, x0 + w), np.random.uniform(y0, y0 + d), h],
    }

    for face, sampler in faces.items():
        for _ in range(n_points_per_face):
            pt = np.array(sampler()) + np.random.normal(0, 0.02, 3)
            points.append(pt)

    return np.array(points)

def generate_ground_plane(size=100, spacing=5):
    """Generate a sparse ground plane."""
    x = np.arange(0, size, spacing)
    y = np.arange(0, size, spacing)
    xx, yy = np.meshgrid(x, y)
    zz = np.zeros_like(xx) + np.random.normal(0, 0.01, size=xx.shape)
    return np.vstack([xx.ravel(), yy.ravel(), zz.ravel()]).T

# Parameters for multiple buildings based on the visual pattern
building_specs = [
    (10, 10, 8, 10, 3.5),
    (22, 12, 6, 8, 2.5),
    (18, 22, 10, 5, 4.0),
    (32, 14, 6, 6, 3.0),
    (40, 22, 12, 6, 3.8),
    (28, 28, 10, 10, 4.5),
]

# Generate buildings
all_buildings = [generate_building(x, y, w, d, h) for (x, y, w, d, h) in building_specs]

# Combine with ground plane
ground = generate_ground_plane(size=60, spacing=3)
point_cloud = np.vstack(all_buildings + [ground])

# Scale (if needed)
# point_cloud *= 1.0  # Already in realistic metric units

# Plot
fig = plt.figure(figsize=(12, 10))
ax = fig.add_subplot(111, projection='3d')
ax.scatter(point_cloud[:, 0], point_cloud[:, 1], point_cloud[:, 2], s=1, c='gray')
ax.set_xlabel("X (m)")
ax.set_ylabel("Y (m)")
ax.set_zlabel("Z (m)")
ax.set_title("Dense 3D Point Cloud of Semi-Rural Compound")
ax.view_init(elev=45, azim=120)
plt.tight_layout()
plt.show()

# Optional: Save as text
for point in point_cloud[:20]:  # First 20
    print("{:.5f} {:.5f} {:.5f}".format(*point))
