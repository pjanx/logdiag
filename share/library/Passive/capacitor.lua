-- Symbol name
local names =
{
	en = "Capacitor",
	cs = "Kondenzátor",
	sk = "Kondenzátor",
	pl = "Kondensator",
	de = "Kondensator"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-2, -1, 2, 1}

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

-- Register the symbol
logdiag.register ("Capacitor", names, area, terminals, render)


