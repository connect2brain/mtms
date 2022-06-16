from setuptools import setup

package_name = 'eeg_processor'

setup(
    name=package_name,
    version='0.0.0',
    packages=[package_name],
    data_files=[
        ('share/ament_index/resource_index/packages',
            ['resource/' + package_name]),
        ('share/' + package_name, ['package.xml']),
    ],
    install_requires=['setuptools'],
    zip_safe=True,
    maintainer='Kim Valén',
    maintainer_email='kim.valen@aalto.fi',
    description='EEG data processor',
    license='TODO',
    tests_require=['pytest'],
    entry_points={
        'console_scripts': [
            'eeg_processor = eeg_processor.eeg_processor:main',
        ],
    },
)
