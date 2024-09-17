// original source code of Illuminance1..Illuminance3
// The API versions are faster (no calling overhead and
// various speedups) but if you want to introduce new
// illumination models this source is the base for further
// experiments

#if 0
#include "c4d.h"

using namespace cinema;

void ShIlluminance1(VolumeData* sd, Vector* diffuse, Vector* specular, Float exponent)
{
	RayLight* lp;
	Int32			i;
	Vector		col;
	Vector64	lv;
	Float			cosa, cosb;
	Bool			nodif, nospec, calc_specular = exponent > 0.0;

	*specular = Vector(0.0);

	if (sd->calc_illum)
	{
		*diffuse = Vector(0.0);
	}
	else
	{
		*diffuse = Vector(1.0);
		return;
	}

	for (i = sd->GetLightCount() - 1; i >= 0; i--)
	{
		lp = sd->GetLight(i);
		if (!sd->IlluminateSurfacePoint(lp, &col, &lv, sd->p, sd->bumpn, sd->n, sd->orign, sd->ray->v, sd->calc_shadow ? ILLUMINATEFLAGS::SHADOW : ILLUMINATEFLAGS::NONE, sd->lhit, sd->raybits, true))
			continue;
		if (lp->ambient)
		{
			*diffuse += col; continue;
		}

		nodif = nospec = false;
		if (lp->lr.object)
			CalcRestrictionInc(&lp->lr, sd->op, nodif, nospec);

		if (lp->arealight)
		{
			// tube and area lights are difficult to calculate
			// if you want to combine area lights with your own lighting models
			// you completely need to rebuild the area light shapes!
			// for standard models use LightCalcSpecial

			Vector dif, spc;
			sd->CalcArea(lp, nodif, nospec, exponent, sd->ray->v, sd->p, sd->bumpn, sd->orign, sd->raybits, true, &dif, &spc);

			*diffuse	+= dif * col;
			*specular += spc * col;
		}
		else
		{
			cosa = Dot(lv, sd->bumpn);
			if (!(lp->nodiffuse || nodif) && sd->cosc * cosa >= 0.0)
			{
				if (lp->contrast != 0.0)
					*diffuse += Pow(Abs(cosa), Exp(-lp->contrast)) * col;
				else
					*diffuse += Abs(cosa) * col;
			}

			if (calc_specular && !(lp->nospecular || nospec))
			{
				cosb = Dot(sd->ray->v, lv - sd->bumpn * (2.0 * cosa));
				if (cosb <= 0.0)
					continue;

				*specular += Pow(cosb, exponent) * col;
			}
		}
	}

	diffuse.ClampMin();	// there can be subtractive light sources
	specular.ClampMin();
}

void ShIlluminance2(VolumeData* sd, Vector* diffuse, Vector* specular, Float exponent, const Vector64& p, const Vector64& n, const Vector64& bumpn, const Vector64& rayv, ILLUMINATEFLAGS calc_shadow, RAYBIT raybits)
{
	// shouldn't be used any longer - this routine does not handle area lights well
	RayLight* lp;
	Int32			i;
	Vector		col;
	Vector64	lv;
	Float			cosa, cosb, cosc;
	Bool			calc_specular = exponent > 0.0;

	*specular = Vector(0.0);
	*diffuse	= Vector(0.0);

	cosc = -Dot(rayv, n);

	for (i = sd->GetLightCount() - 1; i >= 0; i--)
	{
		lp = sd->GetLight(i);
		if (!sd->IlluminateSurfacePoint(lp, &col, &lv, p, bumpn, bumpn, n, rayv, calc_shadow, RayHitID(), raybits, true))
			continue;

		cosa = Dot(lv, bumpn);
		if (cosc * cosa >= 0.0)
			*diffuse += Abs(cosa) * col;

		if (!calc_specular)
			continue;

		cosb = Dot(rayv, lv - bumpn * (2.0 * cosa));
		if (cosb <= 0.0)
			continue;

		*specular += Pow(cosb, exponent) * col;
	}

	diffuse.ClampMin();	// there can be subtractive light sources
	specular.ClampMin();
}

Vector ShIlluminance3(VolumeData* sd, const Vector64& p, ILLUMINATEFLAGS calc_shadow, RAYBIT raybits)
{
	RayLight* lp;
	Int32			i;
	Vector		col, lv, sum = Vector(0.0);

	for (i = sd->GetLightCount() - 1; i >= 0; i--)
	{
		lp = sd->GetLight(i);
		if (!sd->IlluminateAnyPoint(lp, &col, nullptr, p, calc_shadow, raybits))
			continue;
		sum += col;
	}

	sum->ClampMin();	// there can be subtractive light sources

	return sum;
}
#endif
