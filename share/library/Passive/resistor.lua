-- Symbol name
local names =
{
	en = "Resistor",
	cs = "Rezistor",
	sk = "Rezistor",
	pl = "Rezystor",
	de = "Widerstand"
}

local names_adj =
{
	en = "Adjustable resistor",
	cs = "Nastavitelný rezistor",
	sk = "Nastaviteľný rezistor",
	pl = "Rezystor zmienny",
	de = "Einstellwiderstand"
}

local names_pot =
{
	en = "Potentiometer",
	cs = "Potenciometr",
	sk = "Potenciometer",
	pl = "Potencjometr",
	de = "Potentiometer"
}

-- Render area in base units (X1, Y1, X2, Y2)
local area     = {-2, -0.5, 2, 0.5}
local area_adj = {-2, -1.5, 2, 1}
local area_pot = {-2, -2, 2, 0.5}

-- Terminal points
local terminals     = {{-2, 0}, {2, 0}}
local terminals_pot = {{-2, 0}, {2, 0}, {2, -2}}

-- Rendering
local render = function (cr)
	-- The rectangle
	cr.move_to (-1.5, -0.5)
	cr.line_to (1.5, -0.5)
	cr.line_to (1.5, 0.5)
	cr.line_to (-1.5, 0.5)
	cr.line_to (-1.5, -0.5)

	-- The terminals
	cr.move_to (-2, 0)
	cr.line_to (-1.5, 0)

	cr.move_to (1.5, 0)
	cr.line_to (2, 0)

	cr.stroke ()
end

local render_adj = function (cr)
	render (cr)

	-- The arrow
	cr.move_to (-1, 1)
	cr.line_to (1, -1)

	cr.stroke ()

	cr.save ()
	cr.translate (1.5, -1.5)
	cr.rotate (math.atan2 (1, 1))

	cr.move_to (0, 0)
	cr.line_to (0.3, 0.8)
	cr.line_to (-0.3, 0.8)
	cr.close_path ()

	cr.fill ()
	cr.restore ()
end

local render_pot = function (cr)
	render (cr)

	-- The contact
	cr.move_to (0, -2)
	cr.line_to (2, -2)

	-- The arrow
	cr.move_to (0, -2)
	cr.line_to (0, -1)

	cr.stroke ()

	cr.move_to (0, -0.5)
	cr.line_to (0.3, -1.3)
	cr.line_to (-0.3, -1.3)
	cr.close_path ()

	cr.fill ()
end

-- Register the symbol
logdiag.register ("Resistor",
	names,     area,     terminals,     render)
logdiag.register ("ResistorAdjustable",
	names_adj, area_adj, terminals,     render_adj)
logdiag.register ("Potentiometer",
	names_pot, area_pot, terminals_pot, render_pot)


