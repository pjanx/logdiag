-- Symbol names
local names_ac =
{
	en = "AC voltage source",
	cs = "Střídavý zdroj napětí"
}

local names_dc =
{
	en = "DC voltage source",
	cs = "Stejnosměrný zdroj napětí"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area_ac = {-2, -2, 2, 2}
local area_dc = {-2, -2, 3, 2}

-- Terminal points
local terminals = {{-2, 0}, {2, 0}, {0, -2}, {0, 2}}

-- Rendering
local render = function (cr)
	-- The circle
	cr.arc (0, 0, 2, 0, math.pi * 2)

	cr.stroke ()
end

local render_ac = function (cr)
	render (cr)

	-- The AC symbol
	cr.move_to (-1, 0.25)
	cr.curve_to (-0.4, -1.5, 0.4, 1.5, 1, -0.25)

	cr.stroke ()
end

local render_dc = function (cr)
	render (cr)

	-- The DC symbol
	cr.move_to (-1, -0.25)
	cr.line_to (1, -0.25)

	cr.move_to (-1, 0.25)
	cr.line_to (-0.2, 0.25)

	cr.move_to (0.2, 0.25)
	cr.line_to (1, 0.25)

	-- Polarity sign
	cr.move_to (2.6, -0.6)
	cr.line_to (2.6, -1.4)

	cr.move_to (2.2, -1)
	cr.line_to (3.0, -1)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("ACSource", names_ac, area_ac, terminals, render_ac)
logdiag.register ("DCSource", names_dc, area_dc, terminals, render_dc)


