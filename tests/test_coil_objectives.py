from simsgeo import JaxStelleratorSymmetricCylindricalFourierCurve, StelleratorSymmetricCylindricalFourierCurve, CurveLength, LpCurveCurvature, LpCurveTorsion, FourierCurve, MinimumDistance, JaxFourierCurve, RotatedCurve
import numpy as np
np.random.seed(1)
import pytest
from simsgeo import parameters
parameters['jit'] = False

@pytest.fixture(scope='module', params=[True, False])
def rotated(request):
    return request.param

def create_curve(curvetype, rotated):
    rand_scale=0.01
    order = 4
    nquadpoints = 200
    coil = StelleratorSymmetricCylindricalFourierCurve(nquadpoints, order, 2)

    if curvetype == "FourierCurve":
        coil = FourierCurve(nquadpoints, order)
    elif curvetype == "JaxFourierCurve":
        coil = JaxFourierCurve(nquadpoints, order)
    elif curvetype == "StelleratorSymmetricCylindricalFourierCurve":
        coil = StelleratorSymmetricCylindricalFourierCurve(nquadpoints, order, 2)
    elif curvetype == "JaxStelleratorSymmetricCylindricalFourierCurve":
        coil = JaxStelleratorSymmetricCylindricalFourierCurve(nquadpoints, order, 2)
    else:
        print('Could not find' + curvetype)
        assert False
    dofs = np.zeros((coil.num_dofs(), ))
    if curvetype in ["FourierCurve", "JaxFourierCurve"]:
        dofs[1] = 1.
        dofs[2*order+3] = 1.
        dofs[4*order+3] = 1.
    elif curvetype in ["StelleratorSymmetricCylindricalFourierCurve", "JaxStelleratorSymmetricCylindricalFourierCurve"]:
        dofs[0] = 1.
        dofs[1] = 0.1
        dofs[order+1] = 0.1
    else:
        assert False
    coil.set_dofs(dofs)

    dofs = np.asarray(coil.get_dofs())
    coil.set_dofs(dofs + rand_scale * np.random.rand(len(dofs)).reshape(dofs.shape))
    if rotated:
        coil = RotatedCurve(coil, 0.5, flip=False)
    return coil

@pytest.fixture(scope='module', params=["FourierCurve", "JaxFourierCurve", "JaxStelleratorSymmetricCylindricalFourierCurve", "StelleratorSymmetricCylindricalFourierCurve"])
def curve(request, rotated):
    return create_curve(request.param, rotated)

def test_curve_length_taylor_test(curve):
    J = CurveLength(curve)
    J0 = J.J()
    curve_dofs = np.asarray(curve.get_dofs())
    h = 1e-3 * np.random.rand(len(curve_dofs)).reshape(curve_dofs.shape)
    dJ = J.dJ()
    deriv = np.sum(dJ * h)
    err = 1e6
    for i in range(5, 15):
        eps = 0.5**i
        curve.set_dofs(curve_dofs + eps * h)
        Jh = J.J()
        deriv_est = (Jh-J0)/eps
        err_new = np.linalg.norm(deriv_est-deriv)
        print("err_new %s" % (err_new))
        assert err_new < 0.55 * err
        err = err_new

def test_curve_curvature_taylor_test(curve):
    J = LpCurveCurvature(curve, p=2)
    J0 = J.J()
    curve_dofs = np.asarray(curve.get_dofs())
    h = 1e-2 * np.random.rand(len(curve_dofs)).reshape(curve_dofs.shape)
    dJ = J.dJ()
    deriv = np.sum(dJ * h)
    assert np.abs(deriv) > 1e-10
    err = 1e6
    for i in range(5, 15):
        eps = 0.5**i
        curve.set_dofs(curve_dofs + eps * h)
        Jh = J.J()
        deriv_est = (Jh-J0)/eps
        err_new = np.linalg.norm(deriv_est-deriv)
        print("err_new %s" % (err_new))
        assert err_new < 0.55 * err
        err = err_new


def test_curve_torsion_taylor_test(curve):
    J = LpCurveTorsion(curve, p=2)
    J0 = J.J()
    curve_dofs = np.asarray(curve.get_dofs())
    h = 1e-3 * np.random.rand(len(curve_dofs)).reshape(curve_dofs.shape)
    dJ = J.dJ()
    deriv = np.sum(dJ * h)
    assert np.abs(deriv) > 1e-10
    err = 1e6
    for i in range(10, 20):
        eps = 0.5**i
        curve.set_dofs(curve_dofs + eps * h)
        Jh = J.J()
        deriv_est = (Jh-J0)/eps
        err_new = np.linalg.norm(deriv_est-deriv)
        print("err_new %s" % (err_new))
        assert err_new < 0.55 * err
        err = err_new

def test_curve_minimum_distance_taylor_test(curve):
    ncurves = 3
    curve_t = curve.curve.__class__.__name__ if isinstance(curve, RotatedCurve) else curve.__class__.__name__
    curves = [curve] + [RotatedCurve(create_curve(curve_t, False), 0.1*i, True) for i in range(1, ncurves)]
    J = MinimumDistance(curves, 0.2)
    for k in range(ncurves):
        curve_dofs = np.asarray(curves[k].get_dofs())
        h = 1e-3 * np.random.rand(len(curve_dofs)).reshape(curve_dofs.shape)
        J0 = J.J()
        dJ = J.dJ()[k]
        deriv = np.sum(dJ * h)
        assert np.abs(deriv) > 1e-10
        err = 1e6
        for i in range(5, 15):
            eps = 0.5**i
            curves[k].set_dofs(curve_dofs + eps * h)
            Jh = J.J()
            deriv_est = (Jh-J0)/eps
            err_new = np.linalg.norm(deriv_est-deriv)
            print("err_new %s" % (err_new))
            assert err_new < 0.55 * err
            err = err_new

if __name__ == "__main__":
    test_curve_length_taylor_test()
