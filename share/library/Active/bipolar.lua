-- Symbol names
local names_npn =
{
	en = "NPN transistor",
	cs = "Tranzistor NPN",
	sk = "Tranzistor NPN",
	pl = "Tranzystor NPN"
}

local names_pnp =
{
	en = "PNP transistor",
	cs = "Tranzistor PNP",
	sk = "Tranzistor PNP",
	pl = "Tranzystor PNP"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -2, 2, 2}

-- Terminal points
local terminals = {{-2, 0}, {2, 2}, {2, -2}}

-- Rendering
local render = function (cr)
	-- The terminals
	cr.move_to (-2, 0)
	cr.line_to (0, 0)

	cr.move_to (0, 0.5)
	cr.line_to (2, 2)

	cr.move_to (0, -0.5)
	cr.line_to (2, -2)

	-- The ohmic connection
	cr.move_to (0, -1)
	cr.line_to (0, 1)

	cr.stroke ()
end

local render_npn = function (cr)
	render (cr)

	cr.save ()
	cr.translate (0, 0.5)
	cr.rotate (math.atan2 (-2, 1.5))

	cr.move_to (-0.4, 0.8)
	cr.line_to (0, 1.4)
	cr.line_to (0.4, 0.8)

	cr.stroke ()
	cr.restore ()
end

local render_pnp = function (cr)
	render (cr)

	cr.save ()
	cr.translate (2, -2)
	cr.rotate (math.atan2 (2, 1.5))

	cr.move_to (-0.4, 1.3)
	cr.line_to (0, 1.9)
	cr.line_to (0.4, 1.3)

	cr.stroke ()
	cr.restore ()
end

-- Register the symbols
logdiag.register ("NPN", names_npn, area, terminals, render_npn)
logdiag.register ("PNP", names_pnp, area, terminals, render_pnp)


