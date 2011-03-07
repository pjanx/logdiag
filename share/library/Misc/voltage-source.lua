-- Symbol names
local names_ac =
{
	en = "AC voltage source",
	cs = "Střídavý zdroj napětí",
	sk = "Striedavý zdroj napätia",
	pl = "Źródło prądu zmiennego",
	de = "Wechselstromquelle"
}

local names_dc =
{
	en = "DC voltage source",
	cs = "Stejnosměrný zdroj napětí",
	sk = "Stejnosmerný zdroj napätia",
	pl = "Źródło prądu stałego",
	de = "Gleichstromquelle"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -2, 2, 2}

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

	cr.stroke ()
end

-- Register the symbols
logdiag.register ("ACSource", names_ac, area, terminals, render_ac)
logdiag.register ("DCSource", names_dc, area, terminals, render_dc)


