import os
from setuptools import find_packages, setup

# Search for InVesalius's internal packages in invesalius3 directory, allowing
# InVesalius to import them as, e.g., invesalius.gui, instead of invesalius3.invesalius.gui.
#
packages = find_packages(where='invesalius3')
package_dir = {s: 'invesalius3/' + s.replace('.', '/') for s in packages}

packages.append('invesalius3')
packages.append('bridge')

# XXX: setuptools doesn't directly support copying directories while retaining the
#   directory structure: the directory has to be specified for each copied file separately.
#   Therefore, manually generate the list of files to copy by traversing the directory
#   using os.walk().
#
#   Adapted from:
#
#   https://stackoverflow.com/questions/27829754/include-entire-directory-in-python-setup-py-data-files
#
def generate_data_files():
    dirs = (
        ('lib/python3.8/site-packages/locale', 'invesalius3/locale'),
        ('lib/python3.8/site-packages/icons', 'invesalius3/icons'),
        ('lib/python3.8/site-packages/invesalius_cy', 'invesalius3/invesalius_cy'),
        ('lib/python3.8/site-packages/samples', 'invesalius3/samples'),
        ('lib/python3.8/site-packages/navigation', 'invesalius3/navigation'),
    )

    data_files = []
    for dest_dir, source_dir in dirs:
        for dirpath, _, filenames in os.walk(source_dir):
            dirpath_relative = os.path.relpath(dirpath, source_dir)
            install_dir = os.path.join(dest_dir, dirpath_relative)
            list_entry = (install_dir, [os.path.join(dirpath, f) for f in filenames if not f.startswith('.')])
            data_files.append(list_entry)

    data_files.append(('share/ament_index/resource_index/packages', ['resource/neuronavigation_pkg']))
    data_files.append(('share/neuronavigation_pkg', ['package.xml']))

    return data_files

setup(
    name='neuronavigation_pkg',
    version='0.0.1',
    packages=packages,
    package_dir=package_dir,
    install_requires=['setuptools'],
    data_files=generate_data_files(),
    zip_safe=True,
    author='Olli-Pekka Kahilakoski',
    author_email='olli-pekka.kahilakoski@aalto.fi',
    description='Python packages for neuronavigation',
    license='TODO',
    entry_points={
        'console_scripts': [
            "start = bridge.bridge:main",
        ],
    },
)
