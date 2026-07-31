"""Typed models, device boundaries, scan planning, and dataset validation."""

from radiance3d.interfaces import (
    MeasurementAdapter,
    MeasurementReading,
    MeasurementValidity,
    MotionController,
    PositionConfidence,
    PositionKind,
    PositionReport,
)
from radiance3d.models import Angle, AngularStep, HardwareMetadata, RFMeasurement, Sample, Scan
from radiance3d.motion_client import (
    AxisConfiguration,
    AxisState,
    ControllerCommandError,
    DriverCapabilities,
    DriverDiagnostics,
    PhysicalMotionController,
)
from radiance3d.scanning import AxisScan, RasterScanConfig, ScanCoordinator, raster_points
from radiance3d.transport import (
    DeviceIdentityError,
    ProtocolTransport,
    ResponseCorrelationError,
    SerialTransport,
    TransportError,
    TransportTimeout,
)
from radiance3d.validation import ScanValidationError, load_scan

__all__ = [
    "Angle",
    "AngularStep",
    "AxisConfiguration",
    "AxisScan",
    "AxisState",
    "ControllerCommandError",
    "DeviceIdentityError",
    "DriverCapabilities",
    "DriverDiagnostics",
    "HardwareMetadata",
    "MeasurementAdapter",
    "MeasurementReading",
    "MeasurementValidity",
    "MotionController",
    "PositionConfidence",
    "PositionKind",
    "PositionReport",
    "PhysicalMotionController",
    "ProtocolTransport",
    "RFMeasurement",
    "RasterScanConfig",
    "Sample",
    "Scan",
    "ScanCoordinator",
    "ScanValidationError",
    "SerialTransport",
    "ResponseCorrelationError",
    "TransportError",
    "TransportTimeout",
    "load_scan",
    "raster_points",
]

__version__ = "0.1.0.dev0"
