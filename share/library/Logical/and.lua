-- Symbol name
local names =
{
	en = "AND",
	cs = "AND",
	sk = "AND",
	pl = "AND"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area = {-4, -2, 5, 2}

-- Terminal points
local terminals = {{-4, -1}, {-4, 1}, {5, 0}}

-- Rendering
local render = function (cr)
	-- The main shape
	cr.move_to (-2, -2)
	cr.line_to (1, -2)
	cr.arc (1, 0, 2, math.pi * 1.5, math.pi * 0.5)
	cr.line_to (-2, 2)
	cr.close_path ()

	-- The terminals
	cr.move_to (-4, -1)
	cr.line_to (-2, -1)

	cr.move_to (-4, 1)
	cr.line_to (-2, 1)

	cr.move_to (3, 0)
	cr.line_to (5, 0)

	cr.stroke ()
end

-- Register the symbol
logdiag.register ("AND", names, area, terminals, render)


