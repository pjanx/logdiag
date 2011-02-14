-- Symbol name
local names =
{
	en = "Capacitor",
	cs = "Kondenzátor"
}

local names_polar =
{
	en = "Polarized capacitor",
	cs = "Polarizovaný kondenzátor"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area       = {-2, -1, 2, 1}
local area_polar = {-2, -1.5, 2, 1}

-- Terminal points
local terminals = {{-2, 0}, {2, 0}}

-- Rendering
local render = function (cr)
	-- The vertical lines
	cr.move_to (-0.2, -1)
	cr.line_to (-0.2, 1)

	cr.move_to (0.2, -1)
	cr.line_to (0.2, 1)

	-- The terminals
	cr.move_to (-2, 0)
	cr.line_to (-0.2, 0)

	cr.move_to (0.2, 0)
	cr.line_to (2, 0)

	cr.stroke ()
end

local render_polar = function (cr)
	render (cr)

	-- The plus sign
	cr.move_to (-0.6, -1)
	cr.line_to (-1.4, -1)

	cr.move_to (-1, -1.4)
	cr.line_to (-1, -0.6)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("Capacitor",
	names,       area,       terminals, render)
logdiag.register ("CapacitorPolarized",
	names_polar, area_polar, terminals, render_polar)


